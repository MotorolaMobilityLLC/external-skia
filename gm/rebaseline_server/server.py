#!/usr/bin/python

"""
Copyright 2013 Google Inc.

Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.

HTTP server for our HTML rebaseline viewer.
"""

# System-level imports
import argparse
import BaseHTTPServer
import json
import logging
import os
import posixpath
import re
import shutil
import socket
import subprocess
import sys
import thread
import threading
import time
import urlparse

# Imports from within Skia
#
# We need to add the 'tools' directory for svn.py, and the 'gm' directory for
# gm_json.py .
# Make sure that these dirs are in the PYTHONPATH, but add them at the *end*
# so any dirs that are already in the PYTHONPATH will be preferred.
PARENT_DIRECTORY = os.path.dirname(os.path.realpath(__file__))
GM_DIRECTORY = os.path.dirname(PARENT_DIRECTORY)
TRUNK_DIRECTORY = os.path.dirname(GM_DIRECTORY)
TOOLS_DIRECTORY = os.path.join(TRUNK_DIRECTORY, 'tools')
if TOOLS_DIRECTORY not in sys.path:
  sys.path.append(TOOLS_DIRECTORY)
import svn
if GM_DIRECTORY not in sys.path:
  sys.path.append(GM_DIRECTORY)
import gm_json

# Imports from local dir
#
# Note: we import results under a different name, to avoid confusion with the
# Server.results() property. See discussion at
# https://codereview.chromium.org/195943004/diff/1/gm/rebaseline_server/server.py#newcode44
import imagepairset
import results as results_mod

PATHSPLIT_RE = re.compile('/([^/]+)/(.+)')

# A simple dictionary of file name extensions to MIME types. The empty string
# entry is used as the default when no extension was given or if the extension
# has no entry in this dictionary.
MIME_TYPE_MAP = {'': 'application/octet-stream',
                 'html': 'text/html',
                 'css': 'text/css',
                 'png': 'image/png',
                 'js': 'application/javascript',
                 'json': 'application/json'
                 }

# Keys that server.py uses to create the toplevel content header.
# NOTE: Keep these in sync with static/constants.js
KEY__EDITS__MODIFICATIONS = 'modifications'
KEY__EDITS__OLD_RESULTS_HASH = 'oldResultsHash'
KEY__EDITS__OLD_RESULTS_TYPE = 'oldResultsType'

DEFAULT_ACTUALS_DIR = results_mod.DEFAULT_ACTUALS_DIR
DEFAULT_ACTUALS_REPO_REVISION = 'HEAD'
DEFAULT_ACTUALS_REPO_URL = 'http://skia-autogen.googlecode.com/svn/gm-actual'
DEFAULT_PORT = 8888

# Directory within which the server will serve out static files.
STATIC_CONTENTS_DIR = os.path.realpath(os.path.join(PARENT_DIRECTORY, 'static'))
GENERATED_IMAGES_DIR = os.path.join(STATIC_CONTENTS_DIR, 'generated-images')
GENERATED_JSON_DIR = os.path.join(STATIC_CONTENTS_DIR, 'generated-json')

# How often (in seconds) clients should reload while waiting for initial
# results to load.
RELOAD_INTERVAL_UNTIL_READY = 10

_HTTP_HEADER_CONTENT_LENGTH = 'Content-Length'
_HTTP_HEADER_CONTENT_TYPE = 'Content-Type'

_SERVER = None   # This gets filled in by main()


def _run_command(args, directory):
  """Runs a command and returns stdout as a single string.

  Args:
    args: the command to run, as a list of arguments
    directory: directory within which to run the command

  Returns: stdout, as a string

  Raises an Exception if the command failed (exited with nonzero return code).
  """
  logging.debug('_run_command: %s in directory %s' % (args, directory))
  proc = subprocess.Popen(args, cwd=directory,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE)
  (stdout, stderr) = proc.communicate()
  if proc.returncode is not 0:
    raise Exception('command "%s" failed in dir "%s": %s' %
                    (args, directory, stderr))
  return stdout


def _get_routable_ip_address():
  """Returns routable IP address of this host (the IP address of its network
     interface that would be used for most traffic, not its localhost
     interface).  See http://stackoverflow.com/a/166589 """
  sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  sock.connect(('8.8.8.8', 80))
  host = sock.getsockname()[0]
  sock.close()
  return host


def _create_svn_checkout(dir_path, repo_url):
  """Creates local checkout of an SVN repository at the specified directory
  path, returning an svn.Svn object referring to the local checkout.

  Args:
    dir_path: path to the local checkout; if this directory does not yet exist,
              it will be created and the repo will be checked out into it
    repo_url: URL of SVN repo to check out into dir_path (unless the local
              checkout already exists)
  Returns: an svn.Svn object referring to the local checkout.
  """
  local_checkout = svn.Svn(dir_path)
  if not os.path.isdir(dir_path):
    os.makedirs(dir_path)
    local_checkout.Checkout(repo_url, '.')
  return local_checkout


class Server(object):
  """ HTTP server for our HTML rebaseline viewer. """

  def __init__(self,
               actuals_dir=DEFAULT_ACTUALS_DIR,
               actuals_repo_revision=DEFAULT_ACTUALS_REPO_REVISION,
               actuals_repo_url=DEFAULT_ACTUALS_REPO_URL,
               port=DEFAULT_PORT, export=False, editable=True,
               reload_seconds=0):
    """
    Args:
      actuals_dir: directory under which we will check out the latest actual
                   GM results
      actuals_repo_revision: revision of actual-results.json files to process
      actuals_repo_url: SVN repo to download actual-results.json files from
      port: which TCP port to listen on for HTTP requests
      export: whether to allow HTTP clients on other hosts to access this server
      editable: whether HTTP clients are allowed to submit new baselines
      reload_seconds: polling interval with which to check for new results;
                      if 0, don't check for new results at all
    """
    self._actuals_dir = actuals_dir
    self._actuals_repo_revision = actuals_repo_revision
    self._actuals_repo_url = actuals_repo_url
    self._port = port
    self._export = export
    self._editable = editable
    self._reload_seconds = reload_seconds
    self._actuals_repo = _create_svn_checkout(
        dir_path=actuals_dir, repo_url=actuals_repo_url)

    # Reentrant lock that must be held whenever updating EITHER of:
    # 1. self._results
    # 2. the expected or actual results on local disk
    self.results_rlock = threading.RLock()
    # self._results will be filled in by calls to update_results()
    self._results = None

  @property
  def results(self):
    """ Returns the most recently generated results, or None if we don't have
    any valid results (update_results() has not completed yet). """
    return self._results

  @property
  def is_exported(self):
    """ Returns true iff HTTP clients on other hosts are allowed to access
    this server. """
    return self._export

  @property
  def is_editable(self):
    """ Returns true iff HTTP clients are allowed to submit new baselines. """
    return self._editable

  @property
  def reload_seconds(self):
    """ Returns the result reload period in seconds, or 0 if we don't reload
    results. """
    return self._reload_seconds

  def update_results(self, invalidate=False):
    """ Create or update self._results, based on the latest expectations and
    actuals.

    We hold self.results_rlock while we do this, to guarantee that no other
    thread attempts to update either self._results or the underlying files at
    the same time.

    Args:
      invalidate: if True, invalidate self._results immediately upon entry;
                  otherwise, we will let readers see those results until we
                  replace them
    """
    with self.results_rlock:
      if invalidate:
        self._results = None
      logging.info(
          'Updating actual GM results in %s to revision %s from repo %s ...' % (
              self._actuals_dir, self._actuals_repo_revision,
              self._actuals_repo_url))
      self._actuals_repo.Update(path='.', revision=self._actuals_repo_revision)

      # We only update the expectations dir if the server was run with a
      # nonzero --reload argument; otherwise, we expect the user to maintain
      # her own expectations as she sees fit.
      #
      # Because the Skia repo is moving from SVN to git, and git does not
      # support updating a single directory tree, we have to update the entire
      # repo checkout.
      #
      # Because Skia uses depot_tools, we have to update using "gclient sync"
      # instead of raw git (or SVN) update.  Happily, this will work whether
      # the checkout was created using git or SVN.
      if self._reload_seconds:
        logging.info(
            'Updating expected GM results in %s by syncing Skia repo ...' %
            results_mod.DEFAULT_EXPECTATIONS_DIR)
        _run_command(['gclient', 'sync'], TRUNK_DIRECTORY)

      new_results = results_mod.Results(
          actuals_root=self._actuals_dir,
          generated_images_root=GENERATED_IMAGES_DIR,
          diff_base_url=os.path.relpath(
              GENERATED_IMAGES_DIR, GENERATED_JSON_DIR))
      if not os.path.isdir(GENERATED_JSON_DIR):
        os.makedirs(GENERATED_JSON_DIR)
      for summary_type in [results_mod.KEY__HEADER__RESULTS_ALL,
                           results_mod.KEY__HEADER__RESULTS_FAILURES]:
        gm_json.WriteToFile(
            new_results.get_packaged_results_of_type(results_type=summary_type),
            os.path.join(GENERATED_JSON_DIR, '%s.json' % summary_type))

      self._results = new_results

  def _result_loader(self, reload_seconds=0):
    """ Call self.update_results(), either once or periodically.

    Params:
      reload_seconds: integer; if nonzero, reload results at this interval
          (in which case, this method will never return!)
    """
    self.update_results()
    logging.info('Initial results loaded. Ready for requests on %s' % self._url)
    if reload_seconds:
      while True:
        time.sleep(reload_seconds)
        self.update_results()

  def run(self):
    arg_tuple = (self._reload_seconds,)  # start_new_thread needs a tuple,
                                         # even though it holds just one param
    thread.start_new_thread(self._result_loader, arg_tuple)

    if self._export:
      server_address = ('', self._port)
      host = _get_routable_ip_address()
      if self._editable:
        logging.warning('Running with combination of "export" and "editable" '
                        'flags.  Users on other machines will '
                        'be able to modify your GM expectations!')
    else:
      host = '127.0.0.1'
      server_address = (host, self._port)
    http_server = BaseHTTPServer.HTTPServer(server_address, HTTPRequestHandler)
    self._url = 'http://%s:%d' % (host, http_server.server_port)
    logging.info('Listening for requests on %s' % self._url)
    http_server.serve_forever()


class HTTPRequestHandler(BaseHTTPServer.BaseHTTPRequestHandler):
  """ HTTP request handlers for various types of queries this server knows
      how to handle (static HTML and Javascript, expected/actual results, etc.)
  """
  def do_GET(self):
    """
    Handles all GET requests, forwarding them to the appropriate
    do_GET_* dispatcher.

    If we see any Exceptions, return a 404.  This fixes http://skbug.com/2147
    """
    try:
      logging.debug('do_GET: path="%s"' % self.path)
      if self.path == '' or self.path == '/' or self.path == '/index.html' :
        self.redirect_to('/static/index.html')
        return
      if self.path == '/favicon.ico' :
        self.redirect_to('/static/favicon.ico')
        return

      # All requests must be of this form:
      #   /dispatcher/remainder
      # where 'dispatcher' indicates which do_GET_* dispatcher to run
      # and 'remainder' is the remaining path sent to that dispatcher.
      normpath = posixpath.normpath(self.path)
      (dispatcher_name, remainder) = PATHSPLIT_RE.match(normpath).groups()
      dispatchers = {
          'static': self.do_GET_static,
      }
      dispatcher = dispatchers[dispatcher_name]
      dispatcher(remainder)
    except:
      self.send_error(404)
      raise

  def do_GET_static(self, path):
    """ Handle a GET request for a file under the 'static' directory.
    Only allow serving of files within the 'static' directory that is a
    filesystem sibling of this script.

    Args:
      path: path to file (under static directory) to retrieve
    """
    # Strip arguments ('?resultsToLoad=all') from the path
    path = urlparse.urlparse(path).path

    logging.debug('do_GET_static: sending file "%s"' % path)
    full_path = os.path.realpath(os.path.join(STATIC_CONTENTS_DIR, path))
    if full_path.startswith(STATIC_CONTENTS_DIR):
      self.send_file(full_path)
    else:
      logging.error(
          'Attempted do_GET_static() of path [%s] outside of static dir [%s]'
          % (full_path, STATIC_CONTENTS_DIR))
      self.send_error(404)

  def do_POST(self):
    """ Handles all POST requests, forwarding them to the appropriate
        do_POST_* dispatcher. """
    # All requests must be of this form:
    #   /dispatcher
    # where 'dispatcher' indicates which do_POST_* dispatcher to run.
    logging.debug('do_POST: path="%s"' % self.path)
    normpath = posixpath.normpath(self.path)
    dispatchers = {
      '/edits': self.do_POST_edits,
    }
    try:
      dispatcher = dispatchers[normpath]
      dispatcher()
      self.send_response(200)
    except:
      self.send_error(404)
      raise

  def do_POST_edits(self):
    """ Handle a POST request with modifications to GM expectations, in this
    format:

    {
      KEY__EDITS__OLD_RESULTS_TYPE: 'all',  # type of results that the client
                                            # loaded and then made
                                            # modifications to
      KEY__EDITS__OLD_RESULTS_HASH: 39850913, # hash of results when the client
                                              # loaded them (ensures that the
                                              # client and server apply
                                              # modifications to the same base)
      KEY__EDITS__MODIFICATIONS: [
        # as needed by results_mod.edit_expectations()
        ...
      ],
    }

    Raises an Exception if there were any problems.
    """
    if not _SERVER.is_editable:
      raise Exception('this server is not running in --editable mode')

    content_type = self.headers[_HTTP_HEADER_CONTENT_TYPE]
    if content_type != 'application/json;charset=UTF-8':
      raise Exception('unsupported %s [%s]' % (
          _HTTP_HEADER_CONTENT_TYPE, content_type))

    content_length = int(self.headers[_HTTP_HEADER_CONTENT_LENGTH])
    json_data = self.rfile.read(content_length)
    data = json.loads(json_data)
    logging.debug('do_POST_edits: received new GM expectations data [%s]' %
                  data)

    # Update the results on disk with the information we received from the
    # client.
    # We must hold _SERVER.results_rlock while we do this, to guarantee that
    # no other thread updates expectations (from the Skia repo) while we are
    # updating them (using the info we received from the client).
    with _SERVER.results_rlock:
      oldResultsType = data[KEY__EDITS__OLD_RESULTS_TYPE]
      oldResults = _SERVER.results.get_results_of_type(oldResultsType)
      oldResultsHash = str(hash(repr(oldResults[imagepairset.KEY__IMAGEPAIRS])))
      if oldResultsHash != data[KEY__EDITS__OLD_RESULTS_HASH]:
        raise Exception('results of type "%s" changed while the client was '
                        'making modifications. The client should reload the '
                        'results and submit the modifications again.' %
                        oldResultsType)
      _SERVER.results.edit_expectations(data[KEY__EDITS__MODIFICATIONS])

    # Read the updated results back from disk.
    # We can do this in a separate thread; we should return our success message
    # to the UI as soon as possible.
    thread.start_new_thread(_SERVER.update_results, (True,))

  def redirect_to(self, url):
    """ Redirect the HTTP client to a different url.

    Args:
      url: URL to redirect the HTTP client to
    """
    self.send_response(301)
    self.send_header('Location', url)
    self.end_headers()

  def send_file(self, path):
    """ Send the contents of the file at this path, with a mimetype based
        on the filename extension.

    Args:
      path: path of file whose contents to send to the HTTP client
    """
    # Grab the extension if there is one
    extension = os.path.splitext(path)[1]
    if len(extension) >= 1:
      extension = extension[1:]

    # Determine the MIME type of the file from its extension
    mime_type = MIME_TYPE_MAP.get(extension, MIME_TYPE_MAP[''])

    # Open the file and send it over HTTP
    if os.path.isfile(path):
      with open(path, 'rb') as sending_file:
        self.send_response(200)
        self.send_header('Content-type', mime_type)
        self.end_headers()
        self.wfile.write(sending_file.read())
    else:
      self.send_error(404)


def main():
  logging.basicConfig(format='%(asctime)s %(levelname)s %(message)s',
                      datefmt='%m/%d/%Y %H:%M:%S',
                      level=logging.INFO)
  parser = argparse.ArgumentParser()
  parser.add_argument('--actuals-dir',
                    help=('Directory into which we will check out the latest '
                          'actual GM results. If this directory does not '
                          'exist, it will be created. Defaults to %(default)s'),
                    default=DEFAULT_ACTUALS_DIR)
  parser.add_argument('--actuals-repo',
                    help=('URL of SVN repo to download actual-results.json '
                          'files from. Defaults to %(default)s'),
                    default=DEFAULT_ACTUALS_REPO_URL)
  parser.add_argument('--actuals-revision',
                    help=('revision of actual-results.json files to process. '
                          'Defaults to %(default)s .  Beware of setting this '
                          'argument in conjunction with --editable; you '
                          'probably only want to edit results at HEAD.'),
                    default=DEFAULT_ACTUALS_REPO_REVISION)
  parser.add_argument('--editable', action='store_true',
                      help=('Allow HTTP clients to submit new baselines.'))
  parser.add_argument('--export', action='store_true',
                      help=('Instead of only allowing access from HTTP clients '
                            'on localhost, allow HTTP clients on other hosts '
                            'to access this server.  WARNING: doing so will '
                            'allow users on other hosts to modify your '
                            'GM expectations, if combined with --editable.'))
  parser.add_argument('--port', type=int,
                      help=('Which TCP port to listen on for HTTP requests; '
                            'defaults to %(default)s'),
                      default=DEFAULT_PORT)
  parser.add_argument('--reload', type=int,
                      help=('How often (a period in seconds) to update the '
                            'results.  If specified, both expected and actual '
                            'results will be updated by running "gclient sync" '
                            'on your Skia checkout as a whole.  '
                            'By default, we do not reload at all, and you '
                            'must restart the server to pick up new data.'),
                      default=0)
  args = parser.parse_args()
  global _SERVER
  _SERVER = Server(actuals_dir=args.actuals_dir,
                   actuals_repo_revision=args.actuals_revision,
                   actuals_repo_url=args.actuals_repo,
                   port=args.port, export=args.export, editable=args.editable,
                   reload_seconds=args.reload)
  _SERVER.run()


if __name__ == '__main__':
  main()

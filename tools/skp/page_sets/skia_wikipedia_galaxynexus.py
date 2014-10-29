# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0401,W0614


from telemetry.page import page as page_module
from telemetry.page import page_set as page_set_module


class SkiaBuildbotDesktopPage(page_module.Page):

  def __init__(self, url, page_set):
    super(SkiaBuildbotDesktopPage, self).__init__(
        url=url,
        page_set=page_set,
        credentials_path='data/credentials.json')
    self.user_agent_type = 'mobile'
    self.archive_data_file = 'data/skia_wikipedia_galaxynexus.json'

  def RunNavigateSteps(self, action_runner):
    action_runner.NavigateToPage(self)
    action_runner.Wait(5)


class SkiaWikipediaGalaxynexusPageSet(page_set_module.PageSet):

  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaWikipediaGalaxynexusPageSet, self).__init__(
      user_agent_type='mobile',
      archive_data_file='data/skia_wikipedia_galaxynexus.json')

    urls_list = [
      # Why: #6 Alexa most visited worldwide; from robertphillips
      'http://en.m.wikipedia.org/wiki/List_of_The_X-Files_episodes#section_2',
    ]

    for url in urls_list:
      self.AddPage(SkiaBuildbotDesktopPage(url, self))

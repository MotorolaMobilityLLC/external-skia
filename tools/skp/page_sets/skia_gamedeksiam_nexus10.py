# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# pylint: disable=W0401,W0614


from telemetry.page import page as page_module
from telemetry.page import page_set as page_set_module
from telemetry.page import shared_page_state


class SkiaBuildbotDesktopPage(page_module.Page):

  def __init__(self, url, page_set):
    super(SkiaBuildbotDesktopPage, self).__init__(
        url=url,
        page_set=page_set,
        credentials_path='data/credentials.json',
        shared_page_state_class=shared_page_state.Shared10InchTabletPageState)
    self.archive_data_file = 'data/skia_gamedeksiam_nexus10.json'

  def RunNavigateSteps(self, action_runner):
    action_runner.Navigate(self.url)
    action_runner.Wait(5)


class SkiaGamedeksiamNexus10PageSet(page_set_module.PageSet):

  """ Pages designed to represent the median, not highly optimized web """

  def __init__(self):
    super(SkiaGamedeksiamNexus10PageSet, self).__init__(
      archive_data_file='data/skia_gamedeksiam_nexus10.json')

    urls_list = [
      # Why: from Tom W's list.
      'http://game.deksiam.in.th/',
    ]

    for url in urls_list:
      self.AddUserStory(SkiaBuildbotDesktopPage(url, self))

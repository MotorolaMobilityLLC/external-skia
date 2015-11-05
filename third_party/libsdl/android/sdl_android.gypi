# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
# Linux specific files and settings for SDL

{
 #TODO what is really necessary here
 'link_settings': {
   'libraries': [ 
     '-ldl',
     '-lGLESv1_CM',
     '-lGLESv2',
     '-llog',
     '-landroid',
   ],
 },
 'sources': [
   '<(src_dir)/src/SDL.c',
   '<(src_dir)/src/SDL_assert.c',
   '<(src_dir)/src/SDL_error.c',
   '<(src_dir)/src/SDL_hints.c',
   '<(src_dir)/src/SDL_log.c',
   '<(src_dir)/src/atomic/SDL_atomic.c',
   '<(src_dir)/src/atomic/SDL_spinlock.c',
   '<(src_dir)/src/audio/SDL_audio.c',
   '<(src_dir)/src/audio/SDL_audiocvt.c',
   '<(src_dir)/src/audio/SDL_audiodev.c',
   '<(src_dir)/src/audio/SDL_audiotypecvt.c',
   '<(src_dir)/src/audio/SDL_mixer.c',
   '<(src_dir)/src/audio/SDL_wave.c',
   '<(src_dir)/src/audio/dummy/SDL_dummyaudio.c',
   '<(src_dir)/src/audio/android/SDL_androidaudio.c',
   '<(src_dir)/src/cpuinfo/SDL_cpuinfo.c',
   '<(src_dir)/src/dynapi/SDL_dynapi.c',
   '<(src_dir)/src/events/SDL_clipboardevents.c',
   '<(src_dir)/src/events/SDL_dropevents.c',
   '<(src_dir)/src/events/SDL_events.c',
   '<(src_dir)/src/events/SDL_gesture.c',
   '<(src_dir)/src/events/SDL_keyboard.c',
   '<(src_dir)/src/events/SDL_mouse.c',
   '<(src_dir)/src/events/SDL_quit.c',
   '<(src_dir)/src/events/SDL_touch.c',
   '<(src_dir)/src/events/SDL_windowevents.c',
   '<(src_dir)/src/file/SDL_rwops.c',
   '<(src_dir)/src/haptic/SDL_haptic.c',
   '<(src_dir)/src/joystick/SDL_gamecontroller.c',
   '<(src_dir)/src/joystick/SDL_joystick.c',
   '<(src_dir)/src/joystick/android/SDL_sysjoystick.c',
   '<(src_dir)/src/power/SDL_power.c',
   '<(src_dir)/src/power/android/SDL_syspower.c',
   '<(src_dir)/src/loadso/dlopen/SDL_sysloadso.c',
   '<(src_dir)/src/render/SDL_d3dmath.c',
   '<(src_dir)/src/render/SDL_render.c',
   '<(src_dir)/src/render/SDL_yuv_mmx.c',
   '<(src_dir)/src/render/SDL_yuv_sw.c',
   '<(src_dir)/src/render/direct3d/SDL_render_d3d.c',
   '<(src_dir)/src/render/direct3d11/SDL_render_d3d11.c',
   '<(src_dir)/src/render/opengl/SDL_render_gl.c',
   '<(src_dir)/src/render/opengl/SDL_shaders_gl.c',
   '<(src_dir)/src/render/opengles/SDL_render_gles.c',
   '<(src_dir)/src/render/opengles2/SDL_render_gles2.c',
   '<(src_dir)/src/render/opengles2/SDL_shaders_gles2.c',
   '<(src_dir)/src/render/psp/SDL_render_psp.c',
   '<(src_dir)/src/render/software/SDL_blendfillrect.c',
   '<(src_dir)/src/render/software/SDL_blendline.c',
   '<(src_dir)/src/render/software/SDL_blendpoint.c',
   '<(src_dir)/src/render/software/SDL_drawline.c',
   '<(src_dir)/src/render/software/SDL_drawpoint.c',
   '<(src_dir)/src/render/software/SDL_render_sw.c',
   '<(src_dir)/src/render/software/SDL_rotate.c',
   '<(src_dir)/src/stdlib/SDL_getenv.c',
   '<(src_dir)/src/stdlib/SDL_iconv.c',
   '<(src_dir)/src/stdlib/SDL_malloc.c',
   '<(src_dir)/src/stdlib/SDL_qsort.c',
   '<(src_dir)/src/stdlib/SDL_stdlib.c',
   '<(src_dir)/src/stdlib/SDL_string.c',
   '<(src_dir)/src/thread/SDL_thread.c',
   '<(src_dir)/src/timer/SDL_timer.c',
   '<(src_dir)/src/video/SDL_RLEaccel.c',
   '<(src_dir)/src/video/SDL_blit.c',
   '<(src_dir)/src/video/SDL_blit_0.c',
   '<(src_dir)/src/video/SDL_blit_1.c',
   '<(src_dir)/src/video/SDL_blit_A.c',
   '<(src_dir)/src/video/SDL_blit_N.c',
   '<(src_dir)/src/video/SDL_blit_auto.c',
   '<(src_dir)/src/video/SDL_blit_copy.c',
   '<(src_dir)/src/video/SDL_blit_slow.c',
   '<(src_dir)/src/video/SDL_bmp.c',
   '<(src_dir)/src/video/SDL_clipboard.c',
   '<(src_dir)/src/video/SDL_egl.c',
   '<(src_dir)/src/video/SDL_fillrect.c',
   '<(src_dir)/src/video/SDL_pixels.c',
   '<(src_dir)/src/video/SDL_rect.c',
   '<(src_dir)/src/video/SDL_shape.c',
   '<(src_dir)/src/video/SDL_stretch.c',
   '<(src_dir)/src/video/SDL_surface.c',
   '<(src_dir)/src/video/SDL_video.c',
   '<(src_dir)/src/video/android/SDL_androidgl.c',
   '<(src_dir)/src/video/android/SDL_androidkeyboard.c',
   '<(src_dir)/src/video/android/SDL_androidwindow.c',
   '<(src_dir)/src/video/android/SDL_androidmouse.c',
   '<(src_dir)/src/video/android/SDL_androidvideo.c',
   '<(src_dir)/src/video/android/SDL_androidclipboard.c',
   '<(src_dir)/src/video/android/SDL_androidtouch.c',
   '<(src_dir)/src/video/android/SDL_androidevents.c',
   '<(src_dir)/src/video/android/SDL_androidmessagebox.c',
   '<(src_dir)/src/thread/pthread/SDL_systhread.c',
   '<(src_dir)/src/thread/pthread/SDL_syssem.c',
   '<(src_dir)/src/thread/pthread/SDL_sysmutex.c',
   '<(src_dir)/src/thread/pthread/SDL_syscond.c',
   '<(src_dir)/src/thread/pthread/SDL_systls.c',
   '<(src_dir)/src/filesystem/android/SDL_sysfilesystem.c',
   '<(src_dir)/src/timer/unix/SDL_systimer.c',
   '<(src_dir)/src/core/android/SDL_android.c',
   '<(src_dir)/src/haptic/dummy/SDL_syshaptic.c',
   '<(src_dir)/src/main/android/SDL_android_main.c',
 ],
 'defines': [ 
   'GL_GLEXT_PROTOTYPES',
 ],
 'cflags': [
   '-fPIC',
   '-O3',
   '-fvisibility=hidden',
 ],
}

# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'conditions': [
    [ 'os_posix == 1 and OS != "mac" and OS != "ios"', {
      'conditions': [
        ['sysroot!=""', {
          'variables': {
            'pkg-config': '../../../build/linux/pkg-config-wrapper "<(sysroot)" "<(target_arch)" "<(system_libdir)"',
          },
        }, {
          'variables': {
            'pkg-config': 'pkg-config'
          },
        }],
      ],
    }],
  ],

  'targets': [
    {
      'target_name': 'libmmtp',
      # 'type': '<(component)',
      # 'type': 'loadable_module',
      'type': 'static_library',
      'product_name': 'mmtp',
      'sources': [
        'mmtp.cc',
        'DList.cc',
        'Mlist.cc',
        'client.cc',
        'getfile.cc',
        'ini.cc',
        'mmt.cc',
        'ringq.cc',  
        'send.cc',
        'mkdir.cc',
        ],
      'defines': [
        'DEBUG',
      ],
      'msvs_disabled_warnings': [4018, 4244, 4267],
      'variables': {
        'clang_warning_flags_unset': [
          # ssl uses PR_ASSERT(!"foo") instead of PR_ASSERT(false && "foo")
          '-Wstring-conversion',
        ],
      },
      'conditions': [
        ['component == "shared_library"', {
          'conditions': [
            ['OS == "mac" or OS == "ios"', {
              'xcode_settings': {
                'GCC_SYMBOLS_PRIVATE_EXTERN': 'NO',
              },
            }],
            ['os_posix == 1 and OS != "mac" and OS != "ios"', {
              'cflags!': ['-fvisibility=hidden'],
            }],
          ],
        }],
        [ 'clang == 1', {
          'cflags': [
            # There is a broken header guard in /usr/include/nss/secmod.h:
            # https://bugzilla.mozilla.org/show_bug.cgi?id=884072
            '-Wno-header-guard',
          ],
        }],
        [ 'os_posix == 1', {
          'include_dirs': [
            '',
          ],
          'cflags': [
            '',
          ],
          'ldflags': [
            '',
          ],
          'libraries': [
            '-lm',
          ],
        }],
      ],
      'configurations': {
        'Debug_Base': {
          'defines': [
            'DEBUG',
          ],
        },
      },
    },
  ],
}

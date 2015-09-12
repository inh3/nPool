{
    'targets': [

    # npool .node
    {
        'target_name': 'npool',

        # only use windows delayed load hook for iojs versions
        'win_delay_load_hook': "<!(node -e \"var v = process.version.substring(1,2); console.log(v > 0 && v < 4);\")",

        'sources': [
            'npool.cc',
            './source/thread.cc',
            './source/file_manager.cc',
            './source/json_utility.cc',
            './source/callback_queue.cc',
            './source/utilities.cc',
            './source/nrequire.cc',
            './source/isolate_context.cc'
        ],

        'include_dirs': [
            './threadpool',
            './source',
            './source/v8',
            "<!(node -e \"require('nan')\")"
        ],

        'dependencies': [
            'threadpool'
        ],

        'conditions': [
            ['OS=="linux"', {
                'cflags': [
                    '-std=c++0x'
                ]
            }],
            ['OS=="mac"', {
                'cflags': [
                    '-std=c++11',
                    '-stdlib=libc++'
                ]
            }],
             ['OS=="win"', {
                'include_dirs': [
                    './threadpool',
                    './source',
                    './node_modules/nan'
                ],
            }],
        ]
    },

    # thread pool library
    {
        'target_name': 'threadpool',
        'win_delay_load_hook': 'false',

        'type': 'static_library',

        'include_dirs': [
            './threadpool'
        ],

        'sources': [
            './threadpool/synchronize.c',
            './threadpool/task_queue.c',
            './threadpool/thread_pool.c'
        ],

        'conditions': [
            ['OS=="win"', {
                'defines': [
                    '_WIN32'
                ]
            }],
            ['OS=="linux"', {
                'ldflags': [
                    '-pthread'
                ]
            }],
            ['OS=="mac"', {
                'ldflags': [
                    '-pthread'
                ]
            }]
        ]
    }]
}

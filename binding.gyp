{
    'targets': [

    # npool .node
    {
        'target_name': 'npool',
        'sources': [
            'npool.cc',
            './source/thread.cc',
            './source/file_manager.cc',
            './source/json.cc',
            './source/callback_queue.cc',
            './source/utilities.cc'
        ],

        'include_dirs': [
            './threadpool',
            './source'
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
            }]
        ]
    },

    # thread pool library
    {
        'target_name': 'threadpool',
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
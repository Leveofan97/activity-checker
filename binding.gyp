{
  "targets": [
    {
      "target_name": "activityChecker",
      "sources": ["src/activityChecker.cpp"],
      "include_dirs": [
            "<!(node -p \"require('node-addon-api').include_dir\")",
            "../node_modules/node-addon-api",
            "node_modules/node-addon-api",
            "/usr/include/node",
            "/usr/local/include/node",
       ],
      "dependencies": [
        "<!(node -p \"require('node-addon-api').gyp\")"
      ],
      "cflags!": ["-fno-exceptions"],
      "cflags_cc!": ["-fno-exceptions"],
      "conditions": [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': 1
            }
          }
        }]
      ]
    }
  ]
}
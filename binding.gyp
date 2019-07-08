{
  "targets": [
    {
      "target_name": "atomicCounters",
      "sources": [ "src/C/atomicCounters.c" ],
      "conditions": [
        ["OS==\"linux\"", {
          "cflags_cc": [ "-fpermissive", "-Os" ]
        }],
        ["OS=='mac'", {
          "xcode_settings": {
          }
        }],
        ["OS=='win'", {
        }]
      ]
    }
  ]
}

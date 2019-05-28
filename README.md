# GSOC2018_RedHen

To use the Scrolling Ticker Algorithm:

```bash
$ python work.py [your_video_name]
```

To compile the executable from the C source:

```bash
$ gcc -o get_and_display_frames get_and_display_frames.c \
      -lavformat -lavcodec -lswscale -lavutil \
      -lz -lm \
      `sdl-config --cflags --libs` \
      -llept -ltesseract
```

To run from the CLI:

```bash
$ ./get_and_display_frames [video_name]
```

… all commands should be executed from the root of the source tree.

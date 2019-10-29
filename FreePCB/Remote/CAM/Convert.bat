echo off

Drill2Gerber drill_file.drl

Gerber2pdf ^
  -nowarnings -silentexit -output=Output ^
  -background=16,16,16 ^
    -combine ^
      -colour=212,175,55,16   top_copper.grb ^
      -colour=212,175,55      top_solder_mask.grb ^
      -colour=192,192,192     top_silk.grb ^
      -colour=0,0,0           drill_file.drl.grb ^
      -colour=255,255,255     board_outline.grb ^
    -combine -mirror ^
      -colour=212,175,55,16   bottom_copper.grb ^
      -colour=212,175,55      bottom_solder_mask.grb ^
      -colour=192,192,192     bottom_silk.grb ^
      -colour=0,0,0           drill_file.drl.grb ^
      -colour=255,255,255     board_outline.grb ^
  -background=255,255,255 ^
    -combine -nomirror ^
      -colour=0,128,0         top_copper.grb ^
      -colour=135,157,175,200 top_solder_mask.grb ^
      -colour=191,191,0,200   top_silk.grb ^
      -colour=255,255,255     drill_file.drl.grb ^
      -colour=0,0,0           board_outline.grb ^
    -combine -mirror ^
      -colour=0,128,0         bottom_copper.grb ^
      -colour=135,157,175,200 bottom_solder_mask.grb ^
      -colour=191,191,0,200   bottom_silk.grb ^
      -colour=255,255,255     drill_file.drl.grb ^
      -colour=0,0,0           board_outline.grb ^
    -combine -nomirror ^
      -colour=0,128,0         top_copper.grb ^
      -colour=135,157,175,200 top_solder_mask.grb ^
      -colour=191,191,0,200   top_silk.grb ^
      -colour=255,255,255     drill_file.drl.grb ^
      -colour=0,0,0           board_outline.grb ^
    -mirror ^
      -colour=0,128,0         bottom_copper.grb ^
      -colour=135,157,175,200 bottom_solder_mask.grb ^
      -colour=191,191,0,200   bottom_silk.grb ^
      -colour=255,255,255     drill_file.drl.grb ^
      -colour=0,0,0           board_outline.grb ^
    -nomirror -nocombine^
      top_silk.grb ^
      bottom_silk.grb ^
      top_copper.grb ^
      inner_copper_1.grb ^
      inner_copper_2.grb ^
      bottom_copper.grb ^
      top_solder_mask.grb ^
      bottom_solder_mask.grb ^
      drill_file.drl.grb ^
      board_outline.grb 

explorer Output.pdf


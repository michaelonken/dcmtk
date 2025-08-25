# declare executables
DCMTK_ADD_EXECUTABLE(segconv
  segconv.cc
)

# make sure executables are linked to the corresponding libraries
DCMTK_TARGET_LINK_MODULES(segconv dcmseg dcmfg dcmiod dcmdata oflog ofstd)


# Inner Layer 3

So it's not particularly good code quality in `bymltest.cpp` rn, but like its name suggests it's just a test file.

For this test, save Lean's param dictionary in `splatparamdict.txt` like [this](https://github.com/3096/SplatParamPrettyReader/releases/download/Dict/splatparamdict.txt).

Then you can edit a file like

`./bymltest ChargerNormal.bprm mMoveSpeed 4.0`

Or multiple files in a folder like 

`./bymltest dir-to-byml-files mMoveSpeed 4.0`

Be aware that float values must be entered with a decimal point. So instead of just "4", it's necessary to type 4.0 instead.

This project is in early development, so I expect many issues.

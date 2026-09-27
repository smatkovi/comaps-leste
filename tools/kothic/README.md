Kothic Mapcss parser/processor tailored for CoMaps use.

Dependencies:
* Python >= 3.8
      
Python dependencies:
```bash
pip3 install -r requirements.txt
```

## Running unittests

To run all unittests execute next command from project root folder:

```bash
python3 -m unittest discover -s tests
```

this will search for all `test*.py` files within `tests` directory
and execute tests from those files.

## Running integration tests

File `integration-tests/full_drules_gen.py` is intended to generate drules
files for all 6 themes from main CoMaps repo. It could be used to understand
which parts of the project are actually used by CoMaps repo.

Usage:

```shell
cd integration-tests
python3 full_drules_gen.py -d ../../../data -o drules --txt
```

This command will run generation for styles - default light, default dark,
outdoors light, outdoors dark, vehicle light, vehicle dark and put `*.bin`
and `*.txt` files into 'drules' subfolder.

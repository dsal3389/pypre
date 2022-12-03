# pypre
preprocessor written in C, for all languages

## why preprocessor for all languages?
I think the [vision](#vision) section can answer this for now

## vision
in my opinion, when i learned C and the preprocessor step, I sounded like the best feature
that I wished every programming langauge would have, so I decided to create a preprocessor for
python, but now I see that this code can be applied to any code base.

for now I keep on testing it only on python.

vision example:
```py
MAX_PEOPEL = 30

print(MAX_PEOPLE)

# or

python_version = (3, 6)

if python_version[0] == 2:
    def python2_function():
        return 1
```

the vision of this project is to convert those runtime checks to a preprocessor checks

```py

#define MAX_PEOPLE 30

print(MAX_PEOPLE)

# or

#if PYTHON_MAJOR_VERSION == 2:
def python2_function():
    return 1
#endif

```

and all the processed files would be written to to external folders and WONT change the original files

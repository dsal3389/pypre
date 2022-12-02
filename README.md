# pypre
pypre is a python preprocessor written in C

## why python preprocessor?
I think the [vision](#vision) section can answer this for now

## vision
until now, if we wanted to include some code in a very sepecific circumstances, or make constants, we would have to do it on runtime, for example:

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

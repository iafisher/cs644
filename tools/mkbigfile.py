import random
import sys
import string


def word():
    n = random.randint(3, 10)
    return "".join(random.choice(string.ascii_lowercase) for _ in range(n))


def sentence():
    n = random.randint(5, 20)
    words = [word() for _ in range(n)]
    words[0] = words[0].capitalize()
    return " ".join(words) + "."


def paragraph():
    n = random.randint(3, 10)
    return " ".join(sentence() for _ in range(n))


n = int(sys.argv[1]) if len(sys.argv) > 1 else 20000
for i in range(n):
    print(paragraph())
    if i != n - 1:
        print()

#!/usr/bin/env python3
import random
import multiprocessing
import shutil
import os

charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
output = "../resource/big.txt"
GB = 1024 * 1024 * 1024
N = GB / 2 / 8  # 512M / 8B
N = int(N)


# write `n` random words with length `word_len` to `filename`
def gen_random(filename, n, word_len=7):
    with open(filename, "w") as f:
        for _ in range(n):
            word = "".join([random.choice(charset) for x in range(word_len)])
            f.write(word + " ")


cores = multiprocessing.cpu_count()
p = multiprocessing.Pool(cores)

# split to every cores
print("writing...")
n = N // cores
for i in range(cores):
    filename = output + str(i)
    p.apply_async(gen_random, args=(filename, n))
p.close()
p.join()

# combine together
print("combining...")
with open(output, "wb") as wfd:
    for f in [output + str(i) for i in range(cores)]:
        with open(f, "rb") as fd:
            shutil.copyfileobj(fd, wfd)
        os.remove(f)

print("done")

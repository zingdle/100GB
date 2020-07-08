#!/usr/bin/env python3

with open('../resource/advs.txt') as f:
    words = f.read().split()
    words_len = [len(word) for word in words]
    total_length = sum(words_len)
    max_word_length = max(words_len)
    min_word_length = min(words_len)
    word_cnt = len(words)
    word_set = set(words)

print('word cnt:', word_cnt)
print('distinct word cnt', len(word_set))
print('avg word length:', total_length / word_cnt)
print('min word length:', min_word_length)
print('max word length:', max_word_length)

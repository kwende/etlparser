
import os
import csv

good_dir = "C:/Users/ben/Desktop/ETW work/537-GPU-Good/processed"
bad_dir = "C:/Users/ben/Desktop/ETW work/311-bad-gpu/processed"

common_files = []
unmatched_files = []

good_files = set(os.listdir(good_dir))
bad_files = set(os.listdir(bad_dir))
commons = list(good_files & bad_files)

def is_different(list_a, list_b):
    if len(list_a) != len(list_b):
        return True
    else:
        for i in range(0,len(list_a)):
            if(list_a[i] != list_b[i]): 
                return True

    return False

differents = []

for common in commons:

    good_file = os.path.join(good_dir, common)
    bad_file = os.path.join(bad_dir, common)

    good_content = []
    bad_content = []

    with open(good_file) as fin:
        good_content = list(csv.reader(fin))

    with open(bad_file) as fin:
        bad_content = list(csv.reader(fin))

    different = is_different(good_content, bad_content)
    if different:
        differents.append((common, good_content, bad_content))

print('test')
import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 reformat_time.py <data.csv>")

    with open(sys.argv[1], 'r') as fin:
        lines = fin.readlines()

    with open("new_"+sys.argv[1], 'w+') as fout:

        for i in range(1, len(lines)):
            line = lines[i]
            if ",4010" in line:
                fout.write(line)



if __name__=="__main__":
    main()

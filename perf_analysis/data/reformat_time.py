import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 reformat_time.py <data.csv>")

    with open(sys.argv[1], 'r') as fin:
        lines = fin.readlines()

    with open("new_"+sys.argv[1], 'w') as fout:
        header = lines[0].split("real,")[1]
        fout.write(header)

        for i in range(1, len(lines)):
            line = lines[i].split("m")
            minute = line[0][-1]
            seconds = line[1].split("s")[0]
            rest_of_data = line[1].split("s")[1]

            time = round(int(minute)*60 + float(seconds), 3)
            fout.write(str(time)+rest_of_data)


if __name__=="__main__":
    main()

import sys

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 reformat_time.py <data.csv>")

    for x in range(1,4):
        with open("w_int/"+str(x)+".csv", 'r') as w_int:
            with open("no_int/"+str(x)+".csv", 'r') as no_int:
                wlines = w_int.readlines()
                nlines = no_int.readlines()

            with open("combine/"+str(x)+".csv", 'w+') as fout:
                header = wlines[0]
                fout.write(header)

                for i in range(1, len(wlines)):
                    s_wtime = wlines[i].split(",")[0].strip()
                    s_ntime = nlines[i].split(",")[0].strip()

                    print(s_ntime + " | " + s_wtime)
                    wtime = float(s_wtime)
                    ntime = float(s_ntime)

                    improvement = ((ntime-wtime)*100)/ntime

                    amt = wlines[i].split(",")[3]

                    fout.write(str(improvement)+","+amt)


if __name__=="__main__":
    main()

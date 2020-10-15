import csv
import pandas as pd

def main():

	pd.set_option('display.float_format', lambda x: '%.6f' % x)

	df = pd.read_csv("func_times.log")
	df["time"] = df["time"].astype(float)

	insert_avg = df.loc[df["func_name"] == "insert_fb"]["time"].mean()
	append_avg = df.loc[df["func_name"] == "append_write"]["time"].mean()
	fwrite_avg = df.loc[df["func_name"] == "fwrite"]["time"].mean()      
	flush_avg  = df.loc[df["func_name"] == "flush_buf"]["time"].mean()

	print("insert:", format(insert_avg, ".8f"))
	print("append:", format(append_avg, ".8f"))
	print("fwrite:", format(fwrite_avg, ".8f"))
	print("flush: ", format(flush_avg,  ".8f"))


if __name__ == "__main__":
	main()

import csv
import pandas as pd

def main():

	pd.set_option('display.float_format', lambda x: '%.6f' % x)

	df = pd.read_csv("function_timers.log")
	df["time"] = df["time"].astype(float)


	
	frequencies = df["func_name"].value_counts()      

	print("=== Frequencies ===")
	print(frequencies)



	insert_avg = df.loc[df["func_name"] == "insert_fb"]["time"].mean()
	append_avg = df.loc[df["func_name"] == "append_write"]["time"].mean()
	fwrite_avg = df.loc[df["func_name"] == "fwrite"]["time"].mean()      
	flush_write_avg  = df.loc[df["func_name"] == "flush_write"]["time"].mean()
	flush_memset_avg  = df.loc[df["func_name"] == "flush_memset"]["time"].mean()
	user_avg   = df.loc[df["func_name"] == "user_lvl_fwrite"]["time"].mean()      


	print("=== Averages ===")
	print("insert       : ", format(insert_avg, ".8f"))
	print("append       : ", format(append_avg, ".8f"))
	print("int. fwrite  : ", format(fwrite_avg, ".8f"))
	print("flush: write : ", format(flush_write_avg,  ".8f"))
	print("flush: memset: ", format(flush_memset_avg,  ".8f"))
	print("user fwrite  : ", format(user_avg,   ".8f"))

	insert_sum = df.loc[df["func_name"] == "insert_fb"]["time"].sum()
	append_sum = df.loc[df["func_name"] == "append_write"]["time"].sum()
	fwrite_sum = df.loc[df["func_name"] == "fwrite"]["time"].sum()      
	flush_write_sum  = df.loc[df["func_name"] == "flush_write"]["time"].sum()
	flush_memset_sum  = df.loc[df["func_name"] == "flush_memset"]["time"].sum()
	user_sum   = df.loc[df["func_name"] == "user_lvl_fwrite"]["time"].sum()      

	print("=== Sums ===")
	print("insert       : ", format(insert_sum, ".8f"))
	print("append       : ", format(append_sum, ".8f"))
	print("int. fwrite  : ", format(fwrite_sum, ".8f"))
	print("flush: write : ", format(flush_write_sum,  ".8f"))
	print("flush: memset: ", format(flush_memset_sum,  ".8f"))
	print("user fwrite  : ", format(user_sum,   ".8f"))



if __name__ == "__main__":
	main()

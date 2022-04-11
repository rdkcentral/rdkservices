
# This script fixes all broken links in the API reference
# documentation by updating the markdown files after they
# are generated. The script is called from generate_md.sh, but 
# can also be run separately if required.
# 
import os
import glob
import time

flist=glob.glob(os.path.join(r"api/*.md"))
start = time.time()
count = 0
for f in flist:
    with open(f, "r") as rdkfile:
        R2 = rdkfile.read()
        R3 = R2
        list_of_chars = (" [<sup>method</sup>](#head.Methods)"," [<sup>event</sup>](#head.Notifications)","head.","method.","acronym.","term.","event.","ref.")
        count += 1
        print('file count:', count)
        print(f)
        for num in list_of_chars:
            R2 = R2.replace(num,"")
        if R2 == R3:
           print("both files are equal")
        else:
            print("both are different files")
        with open(f, "w") as rdkfile2:
            rdkfile2.writelines(R2)

end =time.time()
print("The time taken to execute the above program is :", end-start)
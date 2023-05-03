
import re
import json
import glob

def run():

    filenames = glob.glob("./wikixml/enwiki-*-abstract.xml")

    if(len(filenames)==0):
        print("no XML files found ...")
        return
    else:
        filenames = sorted(filenames)

    rfile = filenames[-1]
    wfile = filenames[-1].replace(".xml", ".jsonl")
        
    fr = open(rfile, "r")
    fw = open(wfile, "w")

    doc={"title":"", "abstract":""}

    cnt=0

    print(f"reading {rfile} and writing to {wfile}")
    
    while True:

        if (cnt%10000)==0:
            print("\r\tline: ", cnt, end="")
        cnt+=1
        
        line = fr.readline().replace("\n", "").strip()
        if line==None or len(line)==0:
            break

        elif line.startswith("<doc"):
            doc["title"]=""
            doc["abstract"]=""

        elif line.startswith("<title>") and line.endswith("</title>"):
            doc["title"] = line.replace("<title>", "").replace("</title>", "").replace("Wikipedia:", "").strip() 

        elif line.startswith("<abstract>") and line.endswith("</abstract>"):
            doc["abstract"] = line.replace("<abstract>", "").replace("</abstract>", "").strip()             

        elif line.startswith("</doc"):

            if doc["title"]!="" and doc["abstract"]!="" and re.match("[A-Z](.*)\.", doc["abstract"]):
                fw.write(json.dumps(doc)+"\n")

            doc={"title":"", "abstract":""}
                
        else:
            continue
            
    fr.close()
    fw.close()

if __name__ == "__main__":
    run()

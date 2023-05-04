
import re

import copy
import json
import glob

def init_page(page):

    doc={
        "description": {
            "title":"",
            "abstract":"",
            "advanced":{
                "url":"",
                "wiki-id":[],
                "parent-id":[]
            }            
        },
        "main-text":[],
        "tables":[],
        "figures":[]
    }

    mtch = re.findall("((<title>)(.*)(</title>))", page)
    #print(mtch)    
    if len(mtch)>0:
        doc["description"]["title"] = mtch[0][2]

    mtch = re.findall("((<abstract>)(.*)(</abstract>))", page)
    #print(mtch)
    if len(mtch)>0:
        doc["description"]["abstract"] = mtch[2]        

    mtch = re.findall("((<id>)(.*)(</id>))", page)
    #print(mtch)
    if len(mtch)>0:
        for _ in mtch:
            if _[2]==_[2]:
                doc["description"]["advanced"]["wiki-id"].append(int(_[2]))

    mtch = re.findall("((<parentid>)(.*)(</parentid>))", page)
    #print(mtch)
    if len(mtch)>0:
        for _ in mtch:
            if _[2]==_[2]:
                doc["description"]["advanced"]["parent-id"].append(int(_[2]))

    doc["description"]["advanced"]["wiki-id"] = sorted(list(set(doc["description"]["advanced"]["wiki-id"])))
    doc["description"]["advanced"]["parent-id"] = sorted(list(set(doc["description"]["advanced"]["parent-id"])))
        
    return doc
    
def clean_page(page):

    if "#REDIRECT" in page:
        return 0

    doc = init_page(page)
    
    old_page = copy.deepcopy(page)
    new_page = copy.deepcopy(page)
    
    updated = True
    while updated:
        
        updated = False        

        for mtch in re.findall("(\{\{([^\}]*?)\}\})", old_page):

            print("\t -> ", mtch[0])
            if mtch[0].startswith("{{") and mtch[0].endswith("}}"):
                new_page = new_page.replace(mtch[0], " ")
                updated = True

        for mtch in re.findall("(\{\|(([^\}])*?)\|\})", old_page):

            #print("\t -> ", mtch[0])
            if mtch[0].startswith("{|") and mtch[0].endswith("|}"):
                new_page = new_page.replace(mtch[0], " ")
                updated = True                

        for mtch in re.findall("((\&lt\;)((.|\n)*?)(\&gt\;))", old_page):

            #print("\t -> ", mtch)
            if mtch[0].startswith("&lt;") and mtch[0].endswith("&gt;"):
                new_page = new_page.replace(mtch[0], " ")
                updated = True

        for mtch in re.findall("((\[\[)([A-Za-z]+\:)?([^\]]+?)(\([^\]]+?\))?(\|[^\]]+?)*?(\]\]))", old_page):

            #print("match: ", mtch[0], " => ", mtch[3])
            if mtch[0].startswith("[[") and mtch[0].endswith("]]"):
                new_page = new_page.replace(mtch[0], mtch[3])
                updated = True                                

        new_page = new_page.replace("  ", " ")
                
        old_page = copy.deepcopy(new_page)        
        #print(old_page)

        break
    
    #print(new_page)

    new_page = new_page.replace("&quot;", "'")
    new_page = new_page.replace("&nbsp;", " ")
    new_page = new_page.replace("nbsp;", " ")
    new_page = new_page.replace("&amp;", " and ")
    new_page = new_page.replace("  ", " ")
    
    mtch = re.findall("(<text.*>)((.|\n)*)(</text>)", new_page)

    text = mtch[0][1].split("\n")
    for i,item in enumerate(text):

        text = item.strip()

        if len(text)<=1 or (text.startswith("[[") and text.endswith("]]")):
            continue
        elif text.startswith("===") and text.endswith("==="):            
            doc["main-text"].append({"text":text.replace("===", ""),
                                     "name":"subtitle-level-3",
                                     "type":"paragraph"})
        elif text.startswith("==") and text.endswith("=="):            
            doc["main-text"].append({"text":text.replace("==", ""),
                                     "name":"subtitle-level-2",
                                     "type":"paragraph"})
        elif text.startswith("=") and text.endswith("="):            
            doc["main-text"].append({"text":text.replace("=", ""),
                                     "name":"subtitle-level-1",
                                     "type":"paragraph"})
        else:
            #print(f"item [{i}]: ", item, "\n")
            doc["main-text"].append({"text":text, "name":"paragraph", "type":"paragraph"})

    if doc["description"]["title"]=="Ayn Rand":

        print(page)
        
        print(json.dumps(doc, indent=2))

        input("...")
        
    return 1
        
def run():

    filenames = glob.glob("./wikixml/enwiki-*pages-articles1.xml*")

    if(len(filenames)==0):
        print("no XML files found ...")
        return
    else:
        filenames = sorted(filenames)

    rfile = filenames[-1]
    wfile = filenames[-1].split(".xml")[0]+".jsonl"
        
    fr = open(rfile, "r")
    fw = open(wfile, "w")

    print(f"reading {rfile} and writing to {wfile}")

    doc={"description": {"title":"", "abstract":""}, "main-text":[]}

    page=""

    num_pages=0
    while True:
    
        line = fr.readline()#.replace("\n", "").strip()
        #print(line)
        
        if line==None or len(line)==0:
            continue

        elif "<page>" in line:
            page = line
                       
        elif "</page>" in line:
            page += line
            num_pages += clean_page(page)
            
            page=""
            #num_pages += 1
            
        elif len(page)>0:
            page += line

        else:
            continue
    
        if num_pages>16:
            break
        
    fr.close()
    fw.close()

if __name__ == "__main__":
    run()
    

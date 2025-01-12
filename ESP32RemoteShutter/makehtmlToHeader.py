import os 

def indexToHeader(indexPath,onPath,offPath,headerPath):
    if(os.path.exists(indexPath) and os.path.exists(onPath) and os.path.exists(offPath)):
        with open(indexPath, 'r') as f:
            index = f.read()
        with open(onPath,'r') as f: 
            on = f.read()
        with open(offPath,'r') as f:
            off = f.read()

        header = f"""
const char* index_html = R"({index})";

const char* on_html = R"({on})";

const char* off_html = R"({off})";
        """

        with open(headerPath,'w') as f:
            f.write(header)
        print(f'Successfully Converted {indexPath} into {headerPath}')
    else:
        print(f'An HTML Dependency Does Not Exist.')
  
if __name__== '__main__':
    indexPath = './index.html'
    onPath = './on.html'
    offPath = './off.html'
    headerPath = './htmls.h'
    indexToHeader(indexPath,onPath,offPath,headerPath)
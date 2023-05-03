
FT_LANG=./resources/models/fasttext/language/lid.176.bin
if [ -f "$FT_LANG" ]; then
    echo "$FT_LANG exists.\n"
else 
    echo "$FT_LANG does not exist.\n"
    curl https://dl.fbaipublicfiles.com/fasttext/supervised-models/lid.176.bin -o $FT_LANG
fi

ARXIV=./resources/data/arxiv-abstracts/arxiv-abstracts.zip
if [ -f "$ARXIV" ]; then
    echo "$ARXIV exists.\n"
else 
    echo "downloading '$ARXIV'\n"
    open "https://www.kaggle.com/datasets/Cornell-University/arxiv/download" -o $ARXIV
    echo "unzipping '$ARXIV'\n"
    unzip $ARXIV
fi




FT_LANG=./resources/models/fasttext/language/lid.176.bin
if [ -f "$FT_LANG" ]; then
    echo "$FT_LANG exists.\n"
else 
    echo "$FT_LANG does not exist.\n"
    curl https://dl.fbaipublicfiles.com/fasttext/supervised-models/lid.176.bin -o $FT_LANG
fi

CRF_DIR=./resources/models/crf/part-of-speech
CRF_POS=./resources/models/crf/part-of-speech/lapos-0.1.2.tar.gz
if [ -f "$CRF_POS" ]; then
    echo "$CRF_POS exists.\n"
else 
    echo "$CRF_POS does not exist.\n"
    curl https://www.logos.ic.i.u-tokyo.ac.jp/~tsuruoka/lapos/lapos-0.1.2.tar.gz -o $CRF_POS
    cd $CRF_DIR; gunzip lapos-0.1.2.tar.gz
    mv $CRF_DIR/lapos-0.1.2/model_wsj00-18/model.la $CRF_DIR/en/model_wsj00-18/model.la
    mv $CRF_DIR/lapos-0.1.2/model_wsj02-21/model.la $CRF_DIR/en/model_wsj02-21/model.la
    ls -l ./resources/models/crf/part-of-speech/lapos-0.1.2
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



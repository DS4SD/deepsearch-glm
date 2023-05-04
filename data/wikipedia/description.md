# Wikipedia 

## Source data

Go to the main Wikipedia doanload site: https://dumps.wikimedia.org/enwiki

You can get the abstracts by,

```sh
curl https://dumps.wikimedia.org/enwiki/20221101/enwiki-20221101-abstract.xml.gz
```

## Preprocessed data

### links

1. [paper](https://paperswithcode.com/dataset/wikitext-2)
2. [download](https://blog.salesforceairesearch.com/the-wikitext-long-term-dependency-language-modeling-dataset/#download)

### WikiText-2

Introduced by Merity et al. in Pointer Sentinel Mixture Models. The WikiText language modeling dataset is a collection of over 100 million tokens extracted from the set of verified Good and Featured articles on Wikipedia. The dataset is available under the Creative Commons Attribution-ShareAlike License.

Compared to the preprocessed version of Penn Treebank (PTB), WikiText-2 is over 2 times larger and WikiText-103 is over 110 times larger. The WikiText dataset also features a far larger vocabulary and retains the original case, punctuation and numbers - all of which are removed in PTB. As it is composed of full articles, the dataset is well suited for models that can take advantage of long term dependencies.

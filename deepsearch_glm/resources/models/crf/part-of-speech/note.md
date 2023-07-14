# Lookahead Part-Of-Speech Tagger

links:

1. source: https://www.logos.ic.i.u-tokyo.ac.jp/~tsuruoka/lapos/
2. code: https://www.logos.ic.i.u-tokyo.ac.jp/~tsuruoka/lapos/lapos-0.1.2.tar.gz

## Overview

This is a C++ implementation of the part-of-speech (POS) tagging algorithm described in [1]. The tagger is fast (>500 sentences/sec), accurate (97.22% on the WSJ corpus), and trainable with your own POS-annotated corpus. The tagger contains model files trained for English.

## References

[1] Yoshimasa Tsuruoka, Yusuke Miyao, and Jun'ichi Kazama. 2011. Learning with Lookahead: Can History-Based Models Rival Globally Optimized Models? In Proceedings of CoNLL, pp. 238-246.~
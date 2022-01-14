# Parser benchmark results

**Platform**
- Apple Silicon:
  - Apple M1 Pro 
  - Clock Rate	2064 - 3220 MHz
  - Level 1 Cache	2.9 MB
  - Level 2 Cache	28 MB
  - Level 3 Cache	24 MB
- Intel: 
  - Intel(R) Xeon(R) Platinum 8269CY CPU @ 2.50GHz
  - L1d 32K
  - L1i 32K
  - L2 1024K
  - L3 36608K

**Data sets**

Provided at `tests/jsonSample/*`.

**Benchmark parameters**

- Run seconds: 5
- Copies: 1
- Parsers: vpack, rapidjson, simdjson

**Benchmark command**

`build/tools/bench all`

**Results on Apple Silicon**

| DataSet | Parser | Bytes/second | /seccond | 
| ------- | ------ | --------- | ------------ |
|small.json | vpack         |   353864902.56  |     4315425.64 | 
|small.json | rapidjson     |   230053944.32  |     2805535.91 | 
|small.json | simdjson      |   359464035.01  |     4383707.74 | 
|sample.json | vpack         |   995680650.55  |        1448.28 | 
|sample.json | rapidjson     |  1006928504.72  |        1464.64 | 
|sample.json | simdjson      |  5949651057.93  |        8654.15 | 
|sampleNoWhite.json | vpack         |   423805491.36  |        2460.27 | 
|sampleNoWhite.json | rapidjson     |   435326903.29  |        2527.15 | 
|sampleNoWhite.json | simdjson      |  4541360735.75  |       26363.41 | 
|commits.json | vpack         |   336558718.08  |       13347.03 | 
|commits.json | rapidjson     |   359352120.24  |       14250.96 | 
|commits.json | simdjson      |  5330100540.84  |      211377.72 | 
|api-docs.json | vpack         |  1011461861.60  |         838.72 | 
|api-docs.json | rapidjson     |   481196703.45  |         399.01 | 
|api-docs.json | simdjson      |  6983764140.65  |        5791.02 | 
|countries.json | vpack         |   470694962.80  |         415.06 | 
|countries.json | rapidjson     |   355207524.67  |         313.23 | 
|countries.json | simdjson      |  3204614883.33  |        2825.87 | 
|directory-tree.json | vpack         |   384161344.16  |        1290.45 | 
|directory-tree.json | rapidjson     |   352944045.48  |        1185.59 | 
|directory-tree.json | simdjson      |  5515483519.63  |       18527.30 | 
|doubles-small.json | vpack         |   305427083.82  |        1924.48 | 
|doubles-small.json | rapidjson     |  1011800012.58  |        6375.31 | 
|doubles-small.json | simdjson      |  6489715878.52  |       40891.43 | 
|doubles.json | vpack         |   235012467.73  |         197.98 | 
|doubles.json | rapidjson     |   714473841.37  |         601.88 | 
|doubles.json | simdjson      |  5070402082.81  |        4271.39 | 
|file-list.json | vpack         |   474820958.57  |        3137.92 | 
|file-list.json | rapidjson     |   341487276.78  |        2256.77 | 
|file-list.json | simdjson      |  6174334289.11  |       40803.97 | 
|object.json | vpack         |   137135746.15  |         869.15 | 
|object.json | rapidjson     |   497354345.31  |        3152.18 | 
|object.json | simdjson      |  5674623093.22  |       35965.19 | 
|pass1.json | vpack         |   468231704.12  |      324935.26 | 
|pass1.json | rapidjson     |   562717702.39  |      390505.00 | 
|pass1.json | simdjson      |  2861121857.96  |     1985511.35 | 
|pass2.json | vpack         |   112781719.46  |     2168879.22 | 
|pass2.json | rapidjson     |   122575711.81  |     2357225.23 | 
|pass2.json | simdjson      |   211272068.80  |     4062924.40 | 
|pass3.json | vpack         |   821424686.40  |     5550166.80 | 
|pass3.json | rapidjson     |   301773171.45  |     2039007.92 | 
|pass3.json | simdjson      |   537762945.91  |     3633533.42 | 
|random1.json | vpack         |   656035574.05  |       67828.33 | 
|random1.json | rapidjson     |   526461136.63  |       54431.47 | 
|random1.json | simdjson      |  5670261848.60  |      586255.36 | 
|random2.json | vpack         |   658672135.38  |       79945.64 | 
|random2.json | rapidjson     |   519746070.20  |       63083.64 | 
|random2.json | simdjson      |  5528636198.45  |      671032.43 | 
|random3.json | vpack         |   627922226.92  |        8607.22 | 
|random3.json | rapidjson     |   498408861.07  |        6831.92 | 
|random3.json | simdjson      |  6439498439.95  |       88269.14 | 


**Results on Intel Chip**

| DataSet | Parser | ec | document/sec | 
| ------- | ------ | --------- | ------------ |
|small.json | vpack         |   188899782.86  |     2303655.89 | 
|small.json | rapidjson     |   127351861.73  |     1553071.48 | 
|small.json | simdjson      |   260920607.46  |     3181958.63 | 
|sample.json | vpack         |   904174249.95  |        1315.18 | 
|sample.json | rapidjson     |  1413211628.26  |        2055.61 | 
|sample.json | simdjson      |  4950599132.35  |        7200.97 | 
|sampleNoWhite.json | vpack         |   244545692.90  |        1419.63 | 
|sampleNoWhite.json | rapidjson     |   459349740.48  |        2666.61 | 
|sampleNoWhite.json | simdjson      |  4149985092.36  |       24091.40 | 
|commits.json | vpack         |   163986219.85  |        6503.26 | 
|commits.json | rapidjson     |   252490281.96  |       10013.10 | 
|commits.json | simdjson      |  4078092802.08  |      161726.40 | 
|api-docs.json | vpack         |   849059621.72  |         704.05 | 
|api-docs.json | rapidjson     |   653520761.57  |         541.91 | 
|api-docs.json | simdjson      |  6662158225.83  |        5524.34 | 
|countries.json | vpack         |   244453766.46  |         215.56 | 
|countries.json | rapidjson     |   271919886.47  |         239.78 | 
|countries.json | simdjson      |  2603110140.11  |        2295.45 | 
|directory-tree.json | vpack         |   184676956.61  |         620.36 | 
|directory-tree.json | rapidjson     |   223655990.89  |         751.29 | 
|directory-tree.json | simdjson      |  2903582799.94  |        9753.55 | 
|doubles-small.json | vpack         |    83976076.75  |         529.13 | 
|doubles-small.json | rapidjson     |   505519075.98  |        3185.25 | 
|doubles-small.json | simdjson      |  4748838987.22  |       29922.24 | 
|doubles.json | vpack         |    58083903.63  |          48.93 | 
|doubles.json | rapidjson     |   333404631.13  |         280.87 | 
|doubles.json | simdjson      |  4322472494.62  |        3641.32 | 
|file-list.json | vpack         |   329627271.95  |        2178.39 | 
|file-list.json | rapidjson     |   266579913.89  |        1761.73 | 
|file-list.json | simdjson      |  5316260708.28  |       35133.27 | 
|object.json | vpack         |    59581590.89  |         377.62 | 
|object.json | rapidjson     |   385906478.91  |        2445.84 | 
|object.json | simdjson      |  4601729866.12  |       29165.30 | 
|pass1.json | vpack         |   250051310.75  |      173526.24 | 
|pass1.json | rapidjson     |   485213399.57  |      336719.92 | 
|pass1.json | simdjson      |  2061878098.89  |     1430866.13 | 
|pass2.json | vpack         |    73247764.40  |     1408610.85 | 
|pass2.json | rapidjson     |    67899297.43  |     1305755.72 | 
|pass2.json | simdjson      |   138015174.90  |     2654137.98 | 
|pass3.json | vpack         |   629831861.52  |     4255620.69 | 
|pass3.json | rapidjson     |   227191647.91  |     1535078.70 | 
|pass3.json | simdjson      |   390059939.88  |     2635540.13 | 
|random1.json | vpack         |   459378717.52  |       47495.73 | 
|random1.json | rapidjson     |   716353526.80  |       74064.67 | 
|random1.json | simdjson      |  4812340388.09  |      497553.80 | 
|random2.json | vpack         |   454839652.06  |       55205.69 | 
|random2.json | rapidjson     |   735467504.66  |       89266.60 | 
|random2.json | simdjson      |  4596730574.84  |      557923.36 | 
|random3.json | vpack         |   437717347.45  |        5999.99 | 
|random3.json | rapidjson     |   423421851.34  |        5804.04 | 
|random3.json | simdjson      |  5465165253.42  |       74913.51 | 

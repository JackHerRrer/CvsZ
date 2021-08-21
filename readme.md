singleRescue.c : 20830 points : 
    go toward the first human and stay with him

doubleRescue.c : 25280 points : 
    find 2 humans that are within the range of the hero. 
    if found send the hero in between
    else send the hero toward the first human

comboAndSingleRescue.c : 27340 points : 
    go toward the first human
    the hero moves freely around the human as long as he stays within range
    the hero looks for opportunities of combos
    known bug : the score is not incremented as long as the first human is not within range

comboAndDoubleRescue.c : 29270 points : 
    find 2 humans that are within the range of the hero. 
    if found send the hero in between, this is the zone to defend
    else send the hero toward the first human
    the hero moves freely around the zone to defend as long as he stays within range
    the hero looks for opportunities of combos
    known bug : the score is not incremented as long as the first human is not within range

doubleComboAndDoubleRescue.c : 27340 points : 
    find 2 humans that are within the range of the hero. 
    if found send the hero in between, this is the zone to defend
    else send the hero toward the first human
    the hero moves freely around the zone to defend as long as he stays within range
    the hero looks for the best combo in the next 2 turns
    known bug : bugy and too slow




depthFirst.c : :
    explore the graph of possibilities as deep as soon as possible before backtracking

beamsearch.c : : todo
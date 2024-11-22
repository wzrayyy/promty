package main

import (
    "strings"
    "github.com/vishalkuo/bimap"
    "golang.org/x/net/idna"
)

var DOMAINS *bimap.BiMap[string, string] = bimap.NewBiMapFromMap(map[string]string {
    "www.docs.kernel.org":        "ввв.доктора.ядро.орг",
    "www.wiki.kernel.org":        "ввв.грешник.ядро.орг",
    "am.mirrors.kernel.org":      "являются.зеркалами.ядро.орг",
    "ams.source.kernel.org":      "источник.амс.ядро.орг",
    "android.kernel.org":         "андроид.ядро.орг",
    "ap.edge.kernel.org":         "край.ап.ядро.орг",
    "archive.kernel.org":         "архив.ядро.орг",
    "b4.docs.kernel.org":         "доктора.б4.ядро.орг",
    "btrfs.docs.kernel.org":      "доктора.битрифс.ядро.орг",
    "bugzilla.kernel.org":        "бугзила.ядро.орг",
    "cdn.kernel.org":             "раздатчик.ядро.орг",
    "cregit.origin.kernel.org":   "происхождение.крегит.ядро.орг",
    "cxl.docs.kernel.org":        "доктора.схл.ядро.орг",
    "dfw.source.kernel.org":      "источник.дфв.ядро.орг",
    "docs.kernel.org":            "доктора.ядро.орг",
    "erofs.docs.kernel.org":      "доктора.эротическаяфс.ядро.орг",
    "erol.kernel.org":            "эротика.ядро.орг",
    "eu.edge.kernel.org":         "край.евросоюза.ядро.орг",
    "git.kernel.org":             "мерзавец.ядро.орг",
    "ieee1394.docs.kernel.org":   "доктора.иеее1394.ядро.орг",
    "kernel.org":                 "ядро.орг",
    "linux-mmp.docs.kernel.org":  "доктора.линукс-карта.ядро.орг",
    "linux.kernel.org":           "линукс.ядро.орг",
    "lkml.kernel.org":            "саял.ядро.орг",
    "lore.kernel.org":            "уздечка.ядро.орг",
    "mail.kernel.org":            "почта.ядро.орг",
    "media.social.kernel.org":    "социальные.сми.ядро.орг",
    "mirrors.edge.kernel.org":    "край.зеркал.ядро.орг",
    "mirrors.kernel.org":         "зеркала.ядро.орг",
    "na.edge.kernel.org":         "край.на.ядро.орг",
    "nntp.lore.kernel.org":       "уздечка.сппн.ядро.орг",
    "ny.mirrors.kernel.org":      "нью-йоркские.зеркала.ядро.орг",
    "nyc.source.kernel.org":      "источник.нук.ядро.орг",
    "pad.kernel.org":             "клавиатура.ядро.орг",
    "parisc.docs.kernel.org":     "доктора.париж.ядро.орг",
    "patchwork.kernel.org":       "путаница.ядро.орг",
    "people.kernel.org":          "люди.ядро.орг",
    "planet.kernel.org":          "планета.ядро.орг",
    "pop.lore.kernel.org":        "уздечка.популярности.ядро.орг",
    "remail.kernel.org":          "перепочта.ядро.орг",
    "sin.source.kernel.org":      "источник.греха.ядро.орг",
    "smtp.kernel.org":            "пппп.ядро.орг",
    "smtp.lore.kernel.org":       "уздечка.пппп.ядро.орг",
    "smtp.subspace.kernel.org":   "подместо.пппп.ядро.орг",
    "smtp1.kernel.org":           "пппп1.ядро.орг",
    "smtp2.kernel.org":           "пппп2.ядро.орг",
    "smtp3.kernel.org":           "пппп3.ядро.орг",
    "social.kernel.org":          "социальный.ядро.орг",
    "sparse.docs.kernel.org":     "редкие.доктора.ядро.орг",
    "subspace.kernel.org":        "подместо.ядро.орг",
    "sv.mirrors.kernel.org":      "зеркала.резюме.ядро.орг",
    "sy.mirrors.kernel.org":      "сай.отражает.ядро.орг",
    "vger.kernel.org":            "мпук.ядро.орг",
    "wiki.kernel.org":            "грешник.ядро.орг",
    "wireless.docs.kernel.org":   "беспроводные.доктора.ядро.орг",
    "wireless.kernel.org":        "радио.ядро.орг",
    "www.kernel.org":             "ввв.ядро.орг",
    "yul.archive.kernel.org":     "архив.юл.ядро.орг",
});

var WILDCARD_DOMAINS = [][]string{
    {".docs.kernel.org",         ".доктора.ядро.орг"},
    {".wiki.kernel.org",         ".грешник.ядро.орг"},
};


func Kernel2Yadro(str string) (string, bool) {
    if yadro, found := DOMAINS.Get(str); found {
        return yadro, true
    }

    for _, kernel_yadro := range WILDCARD_DOMAINS {
        if stem, found := strings.CutSuffix(str, kernel_yadro[0]); found {
            return stem + kernel_yadro[1], true
        }
    }

    return "", false
}

func Yadro2Kernel(str string, punycode bool) (string, bool) {
    var err error

    if punycode {
        str, err = idna.ToUnicode(str)

        if err != nil {
            return "", false
        }
    }

    if kernel, found := DOMAINS.GetInverse(str); found {
        return kernel, true
    }

    for _, kernel_yadro := range WILDCARD_DOMAINS {
        if stem, found := strings.CutSuffix(str, kernel_yadro[1]); found {
            return stem + kernel_yadro[0], true
        }
    }

    return "", false
}

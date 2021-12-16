import json
import textwrap
from google_trans_new import google_translator


def trans_train(path, output_path):
    file = open(path, 'r', encoding='utf-8')
    data = []
    for line in file.readlines():
        dic = json.loads(line)
        data.append(dic)
    content = []
    res_content = len(data[0]) * [0]
    res_label = []
    translated_json = []
    # 如果读入数据采用注释格式data要为data[0]，因为data这时不是字典是数组的数组
    for each_content in data[0]:
        # 这里将content内容转小写方便后面翻译
        content.append(each_content["content"].lower())
        # 标签因为只有0 1属性不需要转小写
        res_label.append(each_content["label"])
        # 根据不同切割的数据集，数据选择范围手动切换
    for each in range(0, 300):
        print(each+1)
        if len(content[each]) < 5000:
            # timeout默认为5s,但是可能出现5s还未翻译完的情况，将timeout手动设置为10s
            translator = google_translator(timeout=10)
            res_content[each] = translator.translate(text=content[each], lang_tgt='en')
        else:
            print("本条数据content过长，需要进行分割后翻译！")
            Cuted_content = textwrap.wrap(content[each], 3000)
            # 打印切割后的content列表
            print(Cuted_content)
            for index in range(len(Cuted_content)):
                translator = google_translator(timeout=10)
                Cuted_content[index] = translator.translate(text=Cuted_content[index], lang_tgt='en')
            res_content[each] = '\n'.join(Cuted_content)
        print("翻译之后的content内容为:")
        print(res_content[each])
        # 将翻译好的content和label重新放回字典中
        temp = {"content": res_content[each], "label": res_label[each]}
        translated_json.append(temp)
    with open(output_path, 'w') as f:
        json.dump(translated_json, f)
    print("翻译后的train数据集已存入json文件:"+output_path)



if __name__ == "__main__":
    # 用于翻译训练集数据
    path = "../dataset/train/train.json"
    output_path = "../dataset/debug.json"
    trans_train(path, output_path)

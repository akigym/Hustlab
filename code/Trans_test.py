import json
import textwrap
from google_trans_new import google_translator

def trans_test(path, output_path):
    file = open(path, 'r', encoding='utf-8')
    data = []
    for line in file.readlines():
        dic = json.loads(line)
        data.append(dic)
    content = []
    res_content = len(data[0]) * [0]
    translated_json = []
    # 如果读入数据采用注释格式data要为data[0]，因为data这时不是字典是数组的数组
    for each_content in data[0]:
        # 这里将content内容转小写方便后面翻译
        content.append(each_content["content"].lower())
    for each in range(0, 400):
        print(each + 1)
        if len(content[each]) < 5000:
            translator = google_translator(timeout=10)
            res_content[each] = translator.translate(text=content[each], lang_tgt='en')
        else:
            print("本条数据content过长，需要进行分割后翻译！")
            Cuted_content = textwrap.wrap(content[each], 3000)
            print(Cuted_content)
            for index in range(len(Cuted_content)):
                translator = google_translator(timeout=10)
                Cuted_content[index] = translator.translate(text=Cuted_content[index], lang_tgt='en')
            res_content[each] = '\n'.join(Cuted_content)
        print("翻译之后的content内容为:")
        print(res_content[each])
        # 将翻译好的content和label重新放回字典中
        temp = {"content": res_content[each]}
        translated_json.append(temp)
    with open(output_path, 'w') as f:
        json.dump(translated_json, f)
    print("翻译后的test数据集已存入json文件:" + output_path)



if __name__ == "__main__":
    # 用于翻译训练集数据
    path = "../dataset/test/test.json"
    output_path = "../dataset/debug.json"
    trans_test(path, output_path)

import json

if __name__ == "__main__":
    # 合并被划分后翻译的json文件，重新组装成翻译后的train_translated.json
    train_content = []
    test_content = []
    for each in range(1, 7):
        with open("../dataset/train/train_translated_" + str(each) + ".json", 'r') as f:
            data = json.load(f)
        for each_content in data:
            train_content.append(each_content)
    with open("../dataset/train/train_translated.json", 'w') as f:
        json.dump(train_content, f)
    # 合并被划分后翻译的json文件，重新组装成翻译后的test_translated.json
    for each in range(1, 3):
        with open("../dataset/test/test_translated_" + str(each) + ".json", 'r') as f:
            data = json.load(f)
        for each_content in data:
            test_content.append(each_content)
    with open("../dataset/test/test_translated.json", 'w') as f:
        json.dump(test_content, f)
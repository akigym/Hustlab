import json
from sklearn.feature_extraction.text import CountVectorizer
from sklearn.naive_bayes import MultinomialNB
from sklearn.feature_extraction.text import TfidfTransformer


def predict(train_content, train_label, test_content):
    stopwords = []
    with open("../dataset/stopwords.txt", "r") as f:
        line = f.readline().split("\n")[0]
        while line:
            stopwords.append(line)
            line = f.readline().split("\n")[0]
    # 实例化向量化对象
    vector = CountVectorizer(stop_words=stopwords)
    # 将训练集中的新闻向量化
    X_train = vector.fit_transform(train_content)
    # 将测试集中的新闻向量化
    X_test = vector.transform(test_content)
    # 实例化tf-idf对象
    tfidf = TfidfTransformer()
    # 将训练集中的词频向量用tf-idf进行转换
    X_train = tfidf.fit_transform(X_train)
    # 将测试集中的词频向量用tf-idf进行转换
    X_test = tfidf.transform(X_test)

    clf = MultinomialNB(alpha=0.014, fit_prior=False, class_prior=[0.49, 0.52])
    clf.fit(X_train, train_label)
    prediction = clf.predict(X_test)
    print("label标签列表为:")
    print(prediction)
    return prediction


def read_data(path, is_train):
    with open(path, 'r') as f:
        result = json.load(f)
    res_content = []
    res_label = []
    if is_train:
        for each_content in result:
            res_content.append(each_content["content"].lower())
            res_label.append(each_content["label"])
    else:
        for each_content in result:
            res_content.append(each_content["content"].lower())
    # print(res_content)
    print(res_label)
    return res_content, res_label


if __name__ == "__main__":
    path_train = "../dataset/train/train_translated.json"
    train_content, train_label = read_data(path_train, True)
    path_test = "../dataset/test/test_translated.json"
    test_content, _ = read_data(path_test, False)
    res = predict(train_content, train_label, test_content)
    with open("../dataset/label.txt", "w") as f:
        for each in res:
            f.write(str(each) + "\n")

# import numpy as np
import pandas as pd
from sklearn import mixture

print("gmm test start")

if __name__ == '__main__':
    # 读取excel数据
    dataframe = pd.read_excel("fengsu.xlsx")

    # dataArray = np.array(dataArray).reshape(1, -1)
    # dataArray = dataframe[["风速", "功率"]]
    dataArray = dataframe[["风速"]]
    print(dataArray)
    # scikit-learn 处理
    n_components_range = range(1, 5)
    # cv_types = ['spherical', 'tied', 'diag', 'full']
    cv_types = ['full']
    for cv_type in cv_types:
        for n_components in n_components_range:
            # Fit a Gaussian mixture with EM
            gmm = mixture.GaussianMixture(
                n_components=n_components, covariance_type=cv_type)
            gmm.fit(dataArray)
            print("gmm wights", gmm.weights_)
            print("gmm means", gmm.means_)
            print("gmm covariances", gmm.covariances_)

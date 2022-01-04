# BranchPredictor
This is the course project of UCSD CSE240A. See[prannoy/CSE240A](https://github.com/prannoy/CSE240A) for the description of this project. We implemented following branch predictors:

* **G-share(Gs)**. 
* **Tournament(T)**. 
* **Tournament with the global part replaced by G- share(TG)**
* **Tournament with Estimator(TE)**. This predictor maintains a choice Correctness History Table(CHT). The index of this table is the same as the global PHT. The value of the index is the choice predictor’s correctness of its most recent choices. With this CHT, we can estimate the choice predictor’s choice basing on its history choices. In our estimator, we just simply count the number of correct choices. We set a threshold, if the number of correct choices is less than the threshold, we inverse the choice predictor’s choice. Tournament part is just the same as the original Tournament.
* **Tournament with Estimator and G-share replace- ment(TEG)**.

![Tour](/figs/TTETEG.png)

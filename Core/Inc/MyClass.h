//
// Created by TaterLi on 24-11-13.
//

#ifndef MYCLASS_H
#define MYCLASS_H

#ifdef __cplusplus
class MyClass {
public:
    static void doSomething();
};


extern "C" {
#endif
    // C兼容的接口声明
    void MyClass_doSomething();

#ifdef __cplusplus
}
#endif

#endif //MYCLASS_H

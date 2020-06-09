// 类和对象测试

class Student {
    var name
    var Chinese
    var math
    var English

    new (n, c, m, e) {
        name = n
        math = m
        Chinese = c
        English = e
    }

    best() {
        var grades = [English, math, Chinese]
        var tmp
        for i (0..grades.count-2) {
            for j (0..grades.count-i-2) {
                if (grades[j] > grades[j+1]) {
                    tmp = grades[j]
                    grades[j] = grades[j+1]
                    grades[j+1] = tmp
                }
            }
        }
        System.print(name + "最好成绩是:" + grades[grades.count-1].toString)
    }
}

var liKai = Student.new("李凯", 88.8, 77, 61.5)
liKai.best()


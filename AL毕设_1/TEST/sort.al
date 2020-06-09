// 冒泡排序测试

fun sort(nums) {
    var tmp
    for i (0..nums.count-2) {
        for j (0..nums.count-i-2) {
            if (nums[j] > nums[j+1]) {
                tmp = nums[j]
                nums[j] = nums[j+1]
                nums[j+1] = tmp
            }
        }
    }
    for i (nums) {
        System.print(i)
    }
}

var nums = [3, 6, 5, 6, 8, 4]

sort(nums)


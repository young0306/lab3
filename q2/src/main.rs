use std::io;

fn main() {
    // 행렬 크기 입력 받기
    println!("첫 번째 행렬의 행과 열 크기를 입력하세요 (예: 3 3):");
    let (rows1, cols1) = read_matrix_size();
    
    println!("두 번째 행렬의 행과 열 크기를 입력하세요 (예: 3 3):");
    let (rows2, cols2) = read_matrix_size();

    // 행렬 크기가 같은지 확인
    if rows1 != rows2 || cols1 != cols2 {
        println!("오류: 두 행렬의 크기가 같아야 합니다!");
        return;
    }

    // 행렬 입력 받기
    println!("첫 번째 행렬의 원소를 입력하세요:");
    let matrix1 = read_matrix(rows1, cols1);
    
    println!("두 번째 행렬의 원소를 입력하세요:");
    let matrix2 = read_matrix(rows2, cols2);

    // 행렬 덧셈 수행
    let result = add_matrices(&matrix1, &matrix2);

    // 결과 출력
    println!("결과 행렬:");
    print_matrix(&result);
}

fn read_matrix_size() -> (usize, usize) {
    let mut input = String::new();
    io::stdin().read_line(&mut input).expect("입력을 읽을 수 없습니다.");
    let nums: Vec<usize> = input
        .split_whitespace()
        .map(|x| x.parse().expect("숫자를 입력해주세요"))
        .collect();
    (nums[0], nums[1])
}

fn read_matrix(rows: usize, cols: usize) -> Vec<Vec<i32>> {
    let mut matrix = Vec::with_capacity(rows);
    for _ in 0..rows {
        let mut input = String::new();
        io::stdin().read_line(&mut input).expect("입력을 읽을 수 없습니다.");
        let row: Vec<i32> = input
            .split_whitespace()
            .map(|x| x.parse().expect("숫자를 입력해주세요"))
            .collect();
        matrix.push(row);
    }
    matrix
}

fn add_matrices(matrix1: &[Vec<i32>], matrix2: &[Vec<i32>]) -> Vec<Vec<i32>> {
    let rows = matrix1.len();
    let cols = matrix1[0].len();
    let mut result = Vec::with_capacity(rows);

    for i in 0..rows {
        let mut row = Vec::with_capacity(cols);
        for j in 0..cols {
            row.push(matrix1[i][j] + matrix2[i][j]);
        }
        result.push(row);
    }
    result
}

fn print_matrix(matrix: &[Vec<i32>]) {
    for row in matrix {
        for &val in row {
            print!("{} ", val);
        }
        println!();
    }
} 

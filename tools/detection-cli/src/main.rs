use opencv::{
    core::{self, Point, Rect, Scalar, Size, Vector},
    imgcodecs, imgproc,
    prelude::*,
};

fn main() -> opencv::Result<()> {
    // Carregar a imagem
    let img = imgcodecs::imread("input.jpg", imgcodecs::IMREAD_COLOR)?;

    if img.empty() {
        panic!("Não foi possível ler a imagem")
    }

    println!("Imagem carregada: {}x{}", img.cols(), img.rows());

    // Converte a imagem para o espaço de cores HSV pois é mais adequado para detectar cores
    let mut hsv = Mat::default();
    imgproc::cvt_color(&img, &mut hsv, imgproc::COLOR_BGR2HSV, 0)?;

    // Define o limite de cores de branco no papel
    let lower_white = Scalar::new(0.0, 0.0, 200.0, 0.0);
    let upper_white = Scalar::new(180.0, 60.0, 255.0, 0.0);

    let mut white_mask = Mat::default();
    core::in_range(&hsv, &lower_white, &upper_white, &mut white_mask)?;

    imgcodecs::imwrite("01-debug_white_mask.jpg", &white_mask, &Vector::new())?;

    // Encontra os contornos na máscara de branco
    let mut contours: Vector<Vector<Point>> = Vector::new();
    imgproc::find_contours(
        &white_mask,
        &mut contours,
        imgproc::RETR_LIST,
        imgproc::CHAIN_APPROX_SIMPLE,
        Point::new(0, 0),
    )?;

    let img_area = img.rows() as f64 * img.cols() as f64;
    let max_dice_area = img_area * 0.15;

    let mut dice_contours: Vector<Vector<Point>> = Vector::new();

    for c in contours.iter() {
        let area = imgproc::contour_area(&c, false)?;
        let rect = imgproc::bounding_rect(&c)?;
        let aspect = rect.width as f64 / rect.height as f64;

        if area < 1000.0 {
            continue;
        }
        if area > max_dice_area {
            continue;
        }
        if aspect < 0.7 || aspect > 1.3 {
            continue;
        }

        dice_contours.push(c.clone());
    }
    println!("Encontrados {} contornos", dice_contours.len());

    // Desenha contorno na imagem
    let mut contour_img = img.clone();

    let contour_color = Scalar::new(0.0, 0.0, 255.0, 0.0);
    let thickness = 2;

    imgproc::draw_contours(
        &mut contour_img,
        &dice_contours,
        -1,
        contour_color,
        thickness,
        imgproc::LINE_8,
        &core::no_array(),
        i32::MAX,
        Point::new(0, 0),
    )?;

    // Salva imagem contornado
    imgcodecs::imwrite("02-debug_countours.jpg", &contour_img, &Vector::new())?;

    Ok(())
}

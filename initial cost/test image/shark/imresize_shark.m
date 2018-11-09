clear all;
imr = imread('./shark_imR.png');
iml = imread('./shark_iml.png');


imr = imresize(imr,[270 480],'Method','bicubic');
iml = imresize(iml,[270 480],'Method','bicubic');

imwrite(imr,'imr.png');
imwrite(iml,'iml.png');
const gulp = require('gulp');
const { series } = require('gulp');
const inline = require('gulp-inline');
const uglify = require('gulp-uglify');
const minifyCss = require('gulp-minify-css');
const minHTML = require('gulp-htmlmin');

const origin = 'src';
const destination = 'dist';

function clean(cb) {
  // body omitted
  cb();
}

function build(cb) {
  gulp
    .src(`${origin}/index.html`)
    .pipe(
      inline({
        base: `${origin}/`,
        js: uglify,
        css: [minifyCss],
        disabledTypes: [],
        ignore: [],
      })
    )
    .pipe(minHTML({ collapseWhitespace: true }))
    .pipe(gulp.dest(`${destination}/`));

  cb();
}

exports.build = build;
exports.default = series(clean, build);

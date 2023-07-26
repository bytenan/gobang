drop database if exists gobang;
create database if not exists gobang;
use gobang;
create table if not exists user(
    uid int primary key auto_increment,
    username varchar(32) unique key not null,
    password varchar(128) not null,
    total_games int,
    total_wins int,
    score int
);
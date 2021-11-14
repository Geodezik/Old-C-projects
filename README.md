# Graphs

/*Краткое описание*/
_______________________________________________________________________________________________________________________________________________________________

О СЛОЖНОСТИ ПО ВРЕМЕНИ<br />
Пусть ищем пути минимальной длины от X к Y
Чтобы алгоритм был линеен по времени, допустимы два сценария обхода графа (пока считаем, что путь есть):
  1. Особая вершина не встретилась. Прошли от X к Y
  2. Особая вершина встретилась. От неё нужно пройти только по оптимальным путям. В противном случае этот перебор может стать квадратичным или хуже
Для решения проблемы 2:
От конечной вершины по всем предкам начинаем увеличивать стоимости. Чтобы не допустить превышение линейной сложности, будем отмечать посещённые вершины и больше в них стоимость не менять.
На самом деле, тогда стоимости возле вершин, которые находятся "около" X, могут оказаться не совсем правильными (более длинный путь закроет возможность посещения для более короткого, но идущего позже).
Но это не важно, стоимости нам нужны для двух целей, не зависящих от этого: 
  1. Стоимости всех вершин-потомков особой вершины, очевидно, будут вычислены верно. Их используем для поиска оптимальных путей
  2. Если стоимость X была числом, большим или равным числу вершин графа и не уменьшилась, то X и Y лежат в разных компонентах связности графа, а значит пути из X в Y не существует. Верно и обратное.
Из всего вышесказанного следует алгоритм
Чтобы найти еще и пути из Y в X, линейный по времени алгоритм допустимо запустить второй раз

О СЛОЖНОСТИ ПО ПАМЯТИ<br />
struct link. Структура, линейная по занимаемой памяти. Число таких структур постоянно и не зависит от n

struct graph. Пусть в графе n элементов. Максимальная сумма чисел всех потомков всех вершин равна 2n-1
То же можно сказать и о сумме чисел предков. Для подсчёта сложности примем, что на каждый элемент структуры графа приходится
не более (2n-1)//n + 1 потомков и столько же предков. (2n-1)//n < 2. Поэтому затраты по памяти структуры graph не
превосходят О((3+3)*n) = O(6n) в грубой оценке.

struct ans_list. Структура, линейная по занимаемой памяти. Число таких структур постоянно (2) и не зависит от n

Отсюда следует линейность затрат динамической памяти
_______________________________________________________________________________________________________________________________________________________________

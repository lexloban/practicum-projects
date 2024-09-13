# [transport-catalogue](https://github.com/underlater/YandexPracticumStudyProjects/tree/main/TransportCatalogue)
Транспортный справочник. Обработка автобусных маршрутов. Ввод вывод с помощью JSON.

Работа происходит в два этапа:
1. Формируется база маршрутов и остановок, задаются настройки. 
   - `base_requests` вносятся автобусы `"type": "Bus"` и остановки `"type": "Stop"`.
   - `render_settings` задаются настройки для svg изображения карты маршрутов.
   - `routing_settings` задаётся скорость всех автобусов и время ожидания на остановке.
2. Выполняются запросы на получение информации из базы в json `stat_requests`.
   -  `"type": "Bus"` выводит всю информацию об автобусе.
   -  `"type": "Stop"` выводит все автобусы проезжающие через остановку.
   -  `"type": "Route"` строит кратчайший путь от остановки "А" до "Б".
   -  `"type": "Map"` рисует карту маршрутов в svg формате.


### Пример с построением кратчайшего пути
<details><summary>Input JSON</summary>
  
```json
{
  "base_requests": [
    {
      "is_roundtrip": true,
      "name": "297",
      "stops": [
        "Biryulyovo Zapadnoye",
        "Biryulyovo Tovarnaya",
        "Universam",
        "Biryulyovo Zapadnoye"
      ],
      "type": "Bus"
    },
    {
      "is_roundtrip": false,
      "name": "635",
      "stops": [
        "Biryulyovo Tovarnaya",
        "Universam",
        "Prazhskaya"
      ],
      "type": "Bus"
    },
    {
      "latitude": 55.574371,
      "longitude": 37.6517,
      "name": "Biryulyovo Zapadnoye",
      "road_distances": {
        "Biryulyovo Tovarnaya": 2600
      },
      "type": "Stop"
    },
    {
      "latitude": 55.587655,
      "longitude": 37.645687,
      "name": "Universam",
      "road_distances": {
        "Biryulyovo Tovarnaya": 1380,
        "Biryulyovo Zapadnoye": 2500,
        "Prazhskaya": 4650
      },
      "type": "Stop"
    },
    {
      "latitude": 55.592028,
      "longitude": 37.653656,
      "name": "Biryulyovo Tovarnaya",
      "road_distances": {
        "Universam": 890
      },
      "type": "Stop"
    },
    {
      "latitude": 55.611717,
      "longitude": 37.603938,
      "name": "Prazhskaya",
      "road_distances": {},
      "type": "Stop"
    }
  ],
  "render_settings": {
    "bus_label_font_size": 20,
    "bus_label_offset": [
      7,
      15
    ],
    "color_palette": [
      "green",
      [
        255,
        160,
        0
      ],
      "red"
    ],
    "height": 200,
    "line_width": 14,
    "padding": 30,
    "stop_label_font_size": 20,
    "stop_label_offset": [
      7,
      -3
    ],
    "stop_radius": 5,
    "underlayer_color": [
      255,
      255,
      255,
      0.85
    ],
    "underlayer_width": 3,
    "width": 200
  },
  "routing_settings": {
    "bus_velocity": 40,
    "bus_wait_time": 6
  },
  "stat_requests": [
    {
      "id": 1,
      "name": "297",
      "type": "Bus"
    },
    {
      "id": 2,
      "name": "635",
      "type": "Bus"
    },
    {
      "id": 3,
      "name": "Universam",
      "type": "Stop"
    },
    {
      "from": "Biryulyovo Zapadnoye",
      "id": 4,
      "to": "Universam",
      "type": "Route"
    },
    {
      "from": "Biryulyovo Zapadnoye",
      "id": 5,
      "to": "Prazhskaya",
      "type": "Route"
    }
  ]
}
```
</details>

<details><summary>Output JSON</summary>
  
```json
[
    {
        "curvature": 1.42963,
        "request_id": 1,
        "route_length": 5990,
        "stop_count": 4,
        "unique_stop_count": 3
    },
    {
        "curvature": 1.30156,
        "request_id": 2,
        "route_length": 11570,
        "stop_count": 5,
        "unique_stop_count": 3
    },
    {
        "buses": [
            "297",
            "635"
        ],
        "request_id": 3
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 2,
                "time": 5.235,
                "type": "Bus"
            }
        ],
        "request_id": 4,
        "total_time": 11.235
    },
    {
        "items": [
            {
                "stop_name": "Biryulyovo Zapadnoye",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "297",
                "span_count": 2,
                "time": 5.235,
                "type": "Bus"
            },
            {
                "stop_name": "Universam",
                "time": 6,
                "type": "Wait"
            },
            {
                "bus": "635",
                "span_count": 1,
                "time": 6.975,
                "type": "Bus"
            }
        ],
        "request_id": 5,
        "total_time": 24.21
    }
]
```
</details>


### Пример с получением SVG изображения
<details><summary>Input JSON</summary>
  
```json
{
  "base_requests": [
    {
      "type": "Bus",
      "name": "14",
      "stops": [
        "Ulitsa Lizy Chaikinoi",
        "Elektroseti",
        "Ulitsa Dokuchaeva",
        "Ulitsa Lizy Chaikinoi"
      ],
      "is_roundtrip": true
    },
    {
      "type": "Bus",
      "name": "114",
      "stops": [
        "Morskoy vokzal",
        "Rivierskiy most"
      ],
      "is_roundtrip": false
    },
    {
      "type": "Stop",
      "name": "Rivierskiy most",
      "latitude": 43.587795,
      "longitude": 39.716901,
      "road_distances": {
        "Morskoy vokzal": 850
      }
    },
    {
      "type": "Stop",
      "name": "Morskoy vokzal",
      "latitude": 43.581969,
      "longitude": 39.719848,
      "road_distances": {
        "Rivierskiy most": 850
      }
    },
    {
      "type": "Stop",
      "name": "Elektroseti",
      "latitude": 43.598701,
      "longitude": 39.730623,
      "road_distances": {
        "Ulitsa Dokuchaeva": 3000,
        "Ulitsa Lizy Chaikinoi": 4300
      }
    },
    {
      "type": "Stop",
      "name": "Ulitsa Dokuchaeva",
      "latitude": 43.585586,
      "longitude": 39.733879,
      "road_distances": {
        "Ulitsa Lizy Chaikinoi": 2000,
        "Elektroseti": 3000
      }
    },
    {
      "type": "Stop",
      "name": "Ulitsa Lizy Chaikinoi",
      "latitude": 43.590317,
      "longitude": 39.746833,
      "road_distances": {
        "Elektroseti": 4300,
        "Ulitsa Dokuchaeva": 2000
      }
    }
  ],
  "render_settings": {
    "width": 600,
    "height": 400,
    "padding": 50,
    "stop_radius": 5,
    "line_width": 14,
    "bus_label_font_size": 20,
    "bus_label_offset": [
      7,
      15
    ],
    "stop_label_font_size": 20,
    "stop_label_offset": [
      7,
      -3
    ],
    "underlayer_color": [
      255,
      255,
      255,
      0.85
    ],
    "underlayer_width": 3,
    "color_palette": [
      "green",
      [255, 160, 0],
      "red"
    ]
  },
  "routing_settings": {
    "bus_velocity": 40,
    "bus_wait_time": 6
  },
  "stat_requests": [
    {
      "type": "Map",
      "id": 1
    }
  ]
}
```
</details>

<details><summary>Output JSON</summary>
  
```json
[
    {
        "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"99.2283,329.5 50,232.18 99.2283,329.5\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <polyline points=\"550,190.051 279.22,50 333.61,269.08 550,190.051\" fill=\"none\" stroke=\"rgb(255,160,0)\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <text fill=\"rgb(255,160,0)\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">14</text>\n  <circle cx=\"279.22\" cy=\"50\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"99.2283\" cy=\"329.5\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"50\" cy=\"232.18\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"333.61\" cy=\"269.08\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"550\" cy=\"190.051\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"279.22\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Elektroseti</text>\n  <text fill=\"black\" x=\"279.22\" y=\"50\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Elektroseti</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Morskoy vokzal</text>\n  <text fill=\"black\" x=\"99.2283\" y=\"329.5\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Morskoy vokzal</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Rivierskiy most</text>\n  <text fill=\"black\" x=\"50\" y=\"232.18\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Rivierskiy most</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"333.61\" y=\"269.08\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ulitsa Dokuchaeva</text>\n  <text fill=\"black\" x=\"333.61\" y=\"269.08\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ulitsa Dokuchaeva</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ulitsa Lizy Chaikinoi</text>\n  <text fill=\"black\" x=\"550\" y=\"190.051\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ulitsa Lizy Chaikinoi</text>\n</svg>",
        "request_id": 1
    }
]
```
</details>

SVG result:

![SVG result](TransportCatalogue/image.png)

# [spreadsheet](https://github.com/underlater/YandexPracticumStudyProjects/tree/main/spreadsheet)
Электронная таблица. Задает в ячейки текст или формулу `=2+2`.
Формульные ячейки парсятся с помощью ANTLR и могут содержать ссылки на другие ячейки `=A1*B5`. 
Поддерживает кэш, поиск циклических зависимостей в формульных ячейках, обработку исключений, печать рабочей области в консоль.

# [selfmade-stl](https://github.com/underlater/practicum-projects/tree/main/selfmade-stl)
Переписанный vector и optional из стандартной библиотеки шаблонов.

# [search-server](https://github.com/underlater/YandexPracticumStudyProjects/tree/main/SearchServer)
Поисковая система. Поддерживает плюс, минус и стоп-слова.

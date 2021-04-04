IG: IG
Root Elements:
  - Label: Hierarchy Panel
    Position: [0, 0]
    Size: [325, 1017]
    Color: [1, 1, 1, 1]
    FrameColor: [1, 1, 1, 1]
    Flags: 128
    ID: 0
    Pool: 0
  - Label: Inspector Panel
    Position: [1570, 0]
    Size: [350, 1017]
    Color: [1, 1, 1, 1]
    FrameColor: [1, 1, 1, 1]
    Flags: 130
    ID: 0
    Pool: 1
  - Label: Scene
    Position: [325, 0]
    Size: [1245, 283]
    Color: [1, 1, 1, 1]
    FrameColor: [1, 1, 1, 1]
    Flags: 128
    ID: 0
    Pool: 2
  - Label: Skinning Editor
    Position: [325, 283]
    Size: [1245, 734]
    Color: [1, 1, 1, 1]
    FrameColor: [1, 1, 1, 1]
    Flags: 128
    ID: 0
    Pool: 3
Dockspace:
  - ID: 0
    Position: [0, 0]
    Size: [1920, 1017]
    Type: 2
    Windows:
      []
    FirstChild: 1
    SecondChild: 2
  - ID: 1
    Position: [0, 0]
    Size: [325, 1017]
    Type: 0
    Windows:
      - HierarchyIndex: 0
        PoolIndex: 0
    Parent: 0
  - ID: 2
    Position: [325, 0]
    Size: [1595, 1017]
    Type: 2
    Windows:
      []
    Parent: 0
    FirstChild: 3
    SecondChild: 6
  - ID: 3
    Position: [325, 0]
    Size: [1245, 1017]
    Type: 1
    Windows:
      []
    Parent: 2
    FirstChild: 4
    SecondChild: 5
  - ID: 4
    Position: [325, 0]
    Size: [1245, 283]
    Type: 0
    Windows:
      - HierarchyIndex: 0
        PoolIndex: 2
    Parent: 3
  - ID: 5
    Position: [325, 283]
    Size: [1245, 734]
    Type: 0
    Windows:
      - HierarchyIndex: 0
        PoolIndex: 3
    Parent: 3
  - ID: 6
    Position: [1570, 0]
    Size: [350, 1017]
    Type: 0
    Windows:
      - HierarchyIndex: 0
        PoolIndex: 1
    Parent: 2
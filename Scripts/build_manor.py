# -*- coding: utf-8 -*-
"""
build_manor.py — 大型冒险宅邸 (Adventure Manor) 关卡生成器
仅使用项目现有美术资源 (Modular_Rural_Cabin / LrAssets / Docks)。

单位: 厘米 (UE 标准)。模块网格 GRID = 400cm = 4m。

用法（在 UE Python 控制台或 MCP 中）:
    import build_manor; build_manor.stage_env()
    build_manor.stage_shell()   # 主楼外壳
    build_manor.stage_tower()   # 塔楼
    build_manor.stage_roof()    # 屋顶
    build_manor.stage_props()   # 室内道具
    build_manor.stage_outside() # 室外
"""
import unreal

GRID = 400.0                      # 4m 模块
LEVEL_PATH = '/Game/Map/L_Manor'

MOD = '/Game/Modular_Rural_Cabin/Meshes/'
MODULAR = MOD + 'Modular/'
PROPS = MOD + 'Props/'
FOLIAGE = MOD + 'Foliage/'
UNIQUE = MOD + 'Unique/'
CHEST = '/Game/LrAssets/TreasureChest/'

_mesh_cache = {}


# ---------------------------------------------------------------- 工具

def mesh(path):
    """加载 StaticMesh 并缓存。"""
    if path not in _mesh_cache:
        m = unreal.EditorAssetLibrary.load_asset(path)
        if not m:
            unreal.log_warning('MISSING MESH: ' + path)
        _mesh_cache[path] = m
    return _mesh_cache[path]


def bounds_origin(path):
    m = mesh(path)
    return None if not m else m.get_bounds().origin


def world():
    return unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()


def actor_sub():
    return unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def rot(yaw):
    # 注意: unreal.Rotator 的位置参数是 C++ 顺序 (roll, pitch, yaw) —— 一律用 keyword
    return unreal.Rotator(roll=0.0, pitch=0.0, yaw=yaw)


def rot3(yaw, pitch=0.0, roll=0.0):
    return unreal.Rotator(roll=roll, pitch=pitch, yaw=yaw)


def place(path, x, y, z=0.0, yaw=0.0, pitch=0.0, roll=0.0,
          label=None, snap_ground=True, pivot='center'):
    """
    放置一个 StaticMeshActor。

    pivot='center' -> 包围盒中心落在 (x,y,z)，用于模块化构件（保证 4m 网格严丝合缝）
    pivot='base'   -> 包围盒底面落在 z，用于家具/道具（自动落地）
    """
    m = mesh(path)
    if not m:
        return None
    b = m.get_bounds()
    o = b.origin
    r = rot3(yaw, pitch, roll)
    if pivot == 'base':
        # 注意: bounds.origin 是包围盒中心，要落到"底面"必须减去 (中心z - 半高)
        p = unreal.Vector(x, y, z - (o.z - b.box_extent.z))
    else:
        # P = target - R(O)，抵消 pivot 偏移
        cr, sr = _cos_sin(yaw)
        px, py, pz = o.x, o.y, o.z
        p = unreal.Vector(
            x - (px * cr + py * sr),
            y - (-px * sr + py * cr),
            z - pz)
    w = world()
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, unreal.Vector(x, y, z), unreal.Rotator(0, 0, 0),
        transient=False)
    if not actor:
        return None
    actor.set_actor_location(p, False, False)
    actor.set_actor_rotation(r, False)
    actor.static_mesh_component.set_static_mesh(m)
    if actor.static_mesh_component:
        actor.static_mesh_component.set_mobility(unreal.ComponentMobility.STATIC)
    name = label or path.split('/')[-1]
    actor.set_actor_label(name)
    return actor


def _cos_sin(deg):
    import math
    r = math.radians(deg)
    return math.cos(r), math.sin(r)


def spawn_class(cls, loc, label, rot_=None):
    w = world()
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(
        cls, unreal.Vector(*loc), rot_ or unreal.Rotator(), transient=False)
    if a:
        a.set_actor_label(label)
    return a


def tag_existing(tag):
    """给关卡里所有已生成 actor 打标签前缀，便于清理。"""
    n = 0
    for a in actor_sub().get_all_level_actors():
        lbl = a.get_actor_label()
        if not lbl.startswith('MANOR_'):
            a.set_actor_label('MANOR_' + lbl)
            n += 1
    unreal.log('MANOR: tagged %d actors' % n)
    return n


def clear_manor():
    """删除所有 MANOR_ 前缀的 actor。"""
    return _clear_where(lambda a: a.get_actor_label().startswith('MANOR_'))


def clear_structure():
    """只删建筑/道具类 actor，保留 stage_env 建的地面与光照。"""
    return _clear_where(
        lambda a: a.get_actor_label().startswith(('MANOR_W', 'MANOR_P', 'MANOR_R',
                                                  'MANOR_T', 'MANOR_D', 'MANOR_C',
                                                  'MANOR_Prch', 'MANOR_Chest',
                                                  'MANOR_Prop', 'MANOR_Out')))


def clear_prefix(*prefixes):
    """只删指定前缀的 actor（让各 stage 互不干扰，可单独重跑）。"""
    ps = tuple(prefixes)
    return _clear_where(lambda a: a.get_actor_label().startswith(ps))


def clear_all():
    """删除关卡内所有 actor（新关卡默认会带光照，交给 stage_env 重建）。"""
    return _clear_where(lambda a: True)


def _clear_where(pred):
    n = 0
    for a in list(actor_sub().get_all_level_actors()):
        if pred(a):
            unreal.EditorLevelLibrary.destroy_actor(a)
            n += 1
    unreal.log('MANOR: cleared %d actors' % n)
    return n


def les():
    return unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)


def save():
    unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_all_dirty_levels()
    ok = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
    unreal.log('MANOR: saved (%s) -> %s' % (ok, LEVEL_PATH))


def world_name():
    return world().get_path_name()


def open_level(path=LEVEL_PATH):
    """打开关卡并返回 world path，确认真的切换了。"""
    return unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(path)


# ---------------------------------------------------------------- 楼层高度
H1 = 300.0          # 一层墙高 (Wall_4m 实际 302)
H2 = 770.0          # 二层顶 (Wall_Top_4m 高 470)
HALL_Z = H1 / 2.0          # 一层墙心
TOP_Z = (H1 + H2) / 2.0    # 二层墙心


def wall(path, x, y, z=None, yaw=0.0, label='W'):
    """放一段模块化墙。x/y = 墙体厚度方向的中线位置。"""
    if z is None:
        z = HALL_Z
    return place(MODULAR + path, x, y, z, yaw=yaw, label=label, pivot='center')


def pillar(x, y, z=0.0, label='P'):
    """角柱：mesh 原点在柱底角，直接放网格角点。"""
    return place(MODULAR + 'Wall_Pillar', x, y, z, label=label, pivot='base')


def _rot_xyz(x, y, z, pitch, yaw, roll):
    """UE 旋转顺序: 先 Roll(X) -> Pitch(Y) -> Yaw(Z)。"""
    import math
    pr, yr, rr = math.radians(pitch), math.radians(yaw), math.radians(roll)
    x1 = x
    y1 = y * math.cos(rr) - z * math.sin(rr)
    z1 = y * math.sin(rr) + z * math.cos(rr)
    x2 = x1 * math.cos(pr) + z1 * math.sin(pr)
    z2 = -x1 * math.sin(pr) + z1 * math.cos(pr)
    x3 = x2 * math.cos(yr) - y1 * math.sin(yr)
    y3 = x2 * math.sin(yr) + y1 * math.cos(yr)
    return x3, y3, z2


def world_z_range(actor):
    """返回 actor 网格（含旋转缩放）在世界空间的 (zmin, zmax)。"""
    c = (actor.get_component_by_class(unreal.StaticMeshComponent)
         or actor.get_component_by_class(unreal.SkeletalMeshComponent))
    if not c:
        return None
    m = (c.get_editor_property('static_mesh') if isinstance(c, unreal.StaticMeshComponent)
         else c.get_editor_property('skinned_asset'))
    if not m:
        return None
    b = m.get_bounds()
    o, e = b.origin, b.box_extent
    L = actor.get_actor_location()
    R = actor.get_actor_rotation()
    S = actor.get_actor_scale3d()
    zs = []
    for sx in (-1, 1):
        for sy in (-1, 1):
            for sz in (-1, 1):
                px, py, pz = (o.x + sx * e.x) * S.x, (o.y + sy * e.y) * S.y, (o.z + sz * e.z) * S.z
                rx, ry, rz = _rot_xyz(px, py, pz, R.pitch, R.yaw, R.roll)
                zs.append(L.z + rz)
    return min(zs), max(zs)


def snap_z(actor, target_z, mode='bottom'):
    """把 actor 沿 Z 平移，使包围盒底(或中心/顶)落在 target_z。"""
    if not actor:
        return actor
    r = world_z_range(actor)
    if not r:
        return actor
    cur = r[0] if mode == 'bottom' else (r[1] if mode == 'top' else (r[0] + r[1]) / 2.0)
    L = actor.get_actor_location()
    actor.set_actor_location(unreal.Vector(L.x, L.y, L.z + (target_z - cur)), False, True)
    return actor


def place_skm(path, x, y, z=0.0, yaw=0.0, pitch=0.0, label='K', scale=1.0):
    """放置骨骼网格 actor（宝箱 SKM_Box 等）。"""
    a = spawn_class(unreal.SkeletalMeshActor, (x, y, z), label, rot3(yaw, pitch))
    if not a:
        return None
    c = a.get_component_by_class(unreal.SkeletalMeshComponent)
    m = unreal.EditorAssetLibrary.load_asset(path)
    if m and c:
        c.set_skeletal_mesh_asset(m)
    c.set_editor_property('animation_mode', unreal.AnimationMode.ANIMATION_SINGLE_NODE)
    if scale != 1.0:
        a.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    snap_z(a, z, 'bottom')
    return a


# ---------------------------------------------------------------- 主楼尺寸
# 占地 32m x 24m，4m 网格。外墙中线：X=±1600, Y=±1200
GX = [-1400, -1000, -600, -200, 200, 600, 1000, 1400]      # 8 格 X 中心
GY = [-1000, -600, -200, 200, 600, 1000]                  # 6 格 Y 中心
WX, WY = 1600.0, 1200.0                                  # 外墙中线
MX = 400.0                                                 # 内纵墙中线
MY = 0.0                                                   # 内横墙中线


def _m(name, layer):
    """一层用 Wall_Xxx，二层用 Wall_Top_Xxx。"""
    return name if layer == 1 else name.replace('Wall_', 'Wall_Top_', 1)


def _ring(z, layer):
    """绕主楼外围排一圈墙。"""
    out = []
    # 南墙 (y=-WY, 沿 X) yaw=90
    south = ['Wall_Window_4m', 'Wall_4m', 'Wall_Door_4m', 'Wall_Door_4m',
             'Wall_4m', 'Wall_Window_4m', 'Wall_Window_2_4m', 'Wall_Window_4m']
    # 北墙 (y=+WY) yaw=90
    north = ['Wall_Window_4m', 'Wall_Window_4m', 'Wall_4m', 'Wall_Window_2_4m',
             'Wall_Window_4m', 'Wall_4m', 'Wall_Window_4m', 'Wall_Window_4m']
    # 西墙 (x=-WX, 沿 Y) yaw=0
    west = ['Wall_Window_4m', 'Wall_4m', 'Wall_Door_4m', 'Wall_4m',
            'Wall_Window_2_4m', 'Wall_Window_4m']
    # 东墙 (x=+WX, 沿 Y) yaw=0
    east = ['Wall_Window_4m', 'Wall_Window_4m', 'Wall_Door_4m', 'Wall_Window_4m',
            'Wall_4m', 'Wall_Window_2_4m']
    for i, x in enumerate(GX):
        out.append(place(MODULAR + _m(south[i], layer), x, -WY, z, yaw=90,
                         label='MANOR_W%d_S%d' % (layer, i)))
        out.append(place(MODULAR + _m(north[i], layer), x, WY, z, yaw=90,
                         label='MANOR_W%d_N%d' % (layer, i)))
    for j, y in enumerate(GY):
        out.append(place(MODULAR + _m(west[j], layer), -WX, y, z, yaw=0,
                         label='MANOR_W%d_W%d' % (layer, j)))
        out.append(place(MODULAR + _m(east[j], layer), WX, y, z, yaw=0,
                         label='MANOR_W%d_E%d' % (layer, j)))
    return out


def _partitions(z, layer):
    """内部隔墙，分出 大厅 / 餐厅 / 储藏室 / 卧室 四个房间。"""
    out = []
    # 横墙 y=0 (沿 X, 8 格) —— 在 x=-600 和 x=1000 留门
    door_x = {-600, 1000}
    for i, x in enumerate(GX):
        m = 'Wall_Door_4m' if x in door_x else 'Wall_4m'
        out.append(place(MODULAR + _m(m, layer), x, MY, z, yaw=90,
                         label='MANOR_P%d_H%d' % (layer, i)))
    # 纵墙 x=400 (沿 Y, 6 格) —— 在 y=-600 和 y=600 留门
    door_y = {-600, 600}
    for j, y in enumerate(GY):
        m = 'Wall_Door_4m' if y in door_y else 'Wall_4m'
        out.append(place(MODULAR + _m(m, layer), MX, y, z, yaw=0,
                         label='MANOR_P%d_V%d' % (layer, j)))
    return out


def stage_shell():
    """主楼外壳：一层墙 + 角柱 + 二层墙。"""
    clear_prefix('MANOR_W', 'MANOR_Pillar')
    _ring(HALL_Z, 1)
    _partitions(HALL_Z, 1)
    # 四角柱 + 中柱（一层通高）
    for x in (-WX, MX, WX):
        for y in (-WY, MY, WY):
            pillar(x, y, 0, 'MANOR_Pillar_%d_%d' % (x, y))
    # 二层外墙
    _ring(TOP_Z, 2)
    save()
    unreal.log('MANOR: stage_shell done, actors=%d' % count())
    return count()


# ---------------------------------------------------------------- 塔楼

TX = [1000.0, 1400.0]          # 塔楼 2x2 格 X 中心
TY = [600.0, 1000.0]           # 塔楼 2x2 格 Y 中心
T_BASE = 800.0                 # 塔楼墙中线
T_Z = [150.0, 450.0, 750.0]    # 三层墙心


def stage_tower():
    """东北角塔楼：2x2 格，三层高出屋檐，顶上加旗帜。"""
    clear_prefix('MANOR_T')
    for li, z in enumerate(T_Z):
        # 南墙 / 北墙 (沿 X) yaw=90
        for i, x in enumerate(TX):
            m = 'Wall_Door_4m' if (li == 0 and i == 0) else 'Wall_Window_4m'
            place(MODULAR + m, x, T_BASE - 400, z, yaw=90,
                  label='MANOR_T%d_S%d' % (li, i))
            place(MODULAR + 'Wall_Window_4m', x, T_BASE + 400, z, yaw=90,
                  label='MANOR_T%d_N%d' % (li, i))
        # 西墙 / 东墙 (沿 Y) yaw=0
        for j, y in enumerate(TY):
            place(MODULAR + ('Wall_Window_2_4m' if li else 'Wall_Window_4m'),
                  T_BASE - 400, y, z, yaw=0, label='MANOR_T%d_W%d' % (li, j))
            place(MODULAR + 'Wall_Window_4m', T_BASE + 400, y, z, yaw=0,
                  label='MANOR_T%d_E%d' % (li, j))
    # 塔楼四根角柱通高
    for x in (T_BASE - 400, T_BASE + 400):
        for y in (T_BASE - 400, T_BASE + 400):
            for li in range(3):
                pillar(x, y, li * 300.0, 'MANOR_TPill%d_%d_%d' % (li, x, y))
    # 塔顶平台
    place(MODULAR + 'Porch_4x4m', 1200, 800, 900, label='MANOR_TRoofPl_1', pivot='center')
    place(MODULAR + 'Porch_4x4m', 1200, 1200, 900, label='MANOR_TRoofPl_2', pivot='center')
    place(MODULAR + 'Porch_4x4m', 800, 800, 900, label='MANOR_TRoofPl_3', pivot='center')
    place(MODULAR + 'Porch_4x4m', 800, 1200, 900, label='MANOR_TRoofPl_4', pivot='center')
    # 塔顶旗帜
    place(PROPS + 'Flag', 1200, 800, 912, label='MANOR_TFlag', pivot='base')
    # 塔身旗杆位的大木箱（冒险感）
    place(PROPS + 'Wooden_Crate', 1000, 600, 900, label='MANOR_TCrate', pivot='base')
    save()
    unreal.log('MANOR: stage_tower done, actors=%d' % count())
    return count()


# ---------------------------------------------------------------- 门廊 + 屋顶

def stage_porch():
    """南面门廊 + 大门 + 烟囱。"""
    clear_prefix('MANOR_Prch', 'MANOR_Door', 'MANOR_Chimney')
    # 门廊地板：南墙 y=-1200 的大门前两格
    place(MODULAR + 'Porch_4x4m', -600, -1400, 6.4, label='MANOR_Prch_1', pivot='center')
    place(MODULAR + 'Porch_4x4m', -200, -1400, 6.4, label='MANOR_Prch_2', pivot='center')
    place(MODULAR + 'Porch_4x4m', 200, -1400, 6.4, label='MANOR_Prch_3', pivot='center')
    # 门廊顶：Porch_Roof_8x4m 覆盖中间，4x4m 补两侧，底边对齐门廊柱顶
    r1 = place(MODULAR + 'Porch_Roof_8x4m', -400, -1400, 0, yaw=0,
               label='MANOR_PrchRoof_1', pivot='center')
    r2 = place(MODULAR + 'Porch_Roof_4x4m', 400, -1400, 0, yaw=0,
               label='MANOR_PrchRoof_2', pivot='center')
    snap_z(r1, 313, 'bottom')
    snap_z(r2, 313, 'bottom')
    # 门廊柱
    for x in (-800, 0, 800):
        pillar(x, -1600, 6.4, 'MANOR_PrchPill_%d' % x)
    # 两扇大门（南墙 i=2,i=3 -> x=-600,-200）
    place(MODULAR + 'Door_01', -600, -1200, 0, yaw=90, label='MANOR_Door_1', pivot='base')
    place(MODULAR + 'Door_01', -200, -1200, 0, yaw=90, label='MANOR_Door_2', pivot='base')
    # 石烟囱（屋顶北侧）
    place(PROPS + 'Stone_Chimney', -1200, 800, 770, label='MANOR_Chimney_1', pivot='base')
    save()
    unreal.log('MANOR: stage_porch done, actors=%d' % count())
    return count()


ROOF_BASE = 770.0       # 屋顶起始高度（二层墙顶）
ROOF_PIECE = 400.0      # 沿 X 每片宽度


def stage_roof():
    """
    主楼双坡屋顶。Roof_4m 是 4m 宽(X) × 4.9m 深(Y) × 2.2m 斜板，
    原始坡角约 24°，加 pitch=±10 把坡度压到约 14°。
    每坡 3 排，排间距 600cm > 板的水平投影 475cm，靠重叠封缝。
    """
    clear_prefix('MANOR_Roof')
    p = MODULAR + 'Roof_4m'
    dz = 153.0
    # 南坡: y -1200 -> 0, yaw=0   沿 +Y 上升
    # 北坡: y  1200 -> 0, yaw=180 沿 -Y 上升
    for side, ys, yaw, pit in (('S', (-1200.0, -600.0, 0.0), 0, 10.0),
                               ('N', (1200.0, 600.0, 0.0), 180, -10.0)):
        for row, y in enumerate(ys):
            z = ROOF_BASE + row * dz
            for i, x in enumerate(GX):
                a = place(p, x, y, z, yaw=yaw, pitch=pit,
                          label='MANOR_Roof%s_%d_%d' % (side, row, i),
                          pivot='center')
                snap_z(a, z, 'bottom')
    save()
    unreal.log('MANOR: stage_roof done, actors=%d' % count())
    return count()


# ---------------------------------------------------------------- 室内道具

class _Kit(object):
    """道具摆放小助手：自动编号 + 自动落地。"""

    def __init__(self, prefix):
        self.pre = prefix
        self.n = 0

    def add(self, mesh, x, y, yaw=0.0, z=0.0, pitch=0.0, prop='Props'):
        self.n += 1
        return place(MOD + prop + '/' + mesh, x, y, z, yaw=yaw, pitch=pitch,
                     label='%s_%03d_%s' % (self.pre, self.n, mesh[:12]), pivot='base')


def stage_props():
    """四个房间按功能塞满道具。"""
    clear_prefix('MANOR_Prop', 'MANOR_Chest')
    k = _Kit('MANOR_Prop')

    # ---------- 大厅 (x -1600..400, y -1200..0)：正厅 + 宝藏 ----------
    k.add('Wooden_Crate', -1300, -1000, 30)
    k.add('Wooden_Crate', -1190, -880, -20)
    k.add('Wooden_Crate', -1300, -950, 15, 78)              # 叠在箱上
    k.add('Metal_Barrel', -1420, -760, 0)
    k.add('Metal_Barrel', -1370, -640, 0)
    k.add('Picnic_Table', -600, -620, 0)
    k.add('Rocking_Chair', -820, -700, 90)
    k.add('Office_Chair', -380, -520, 200)
    k.add('Flag', -220, -260, 0)
    k.add('Pumpkin_1', -320, -820, 0)
    k.add('Cutting_Board', -560, -480, 15)
    k.add('Bowl', -600, -500, 0)
    k.add('Mug', -520, -520, 0)
    k.add('Log_Pile_1', -200, -1050, 0)
    place_skm(CHEST + 'SKM_Box', -1450, -380, 0, 35, label='MANOR_Chest_Hall')

    # ---------- 餐厅 (x 400..1600, y -1200..0)：桌椅 + 餐具 ----------
    k.add('Picnic_Table', 1000, -600, 0)
    k.add('Office_Chair', 850, -600, 90)
    k.add('Office_Chair', 1150, -600, -90)
    k.add('Plastic_Chair', 1000, -380, 180)
    k.add('Plate_1', 950, -560, 0)
    k.add('Plate_2', 1055, -565, 10)
    k.add('Bowl', 1000, -650, 0)
    k.add('Mug', 900, -640, 0)
    k.add('Mug', 1105, -635, 0)
    k.add('Spoon', 975, -570, 5)
    k.add('Fork', 1020, -570, -5)
    k.add('Knive', 1100, -640, 0)
    k.add('Tray', 1000, -500, 0)
    k.add('Wooden_Crate', 1420, -1020, 15)
    k.add('Pumpkin_2', 1350, -220, 0)
    k.add('Metal_Barrel', 600, -1080, 0)

    # ---------- 储藏室 (x -1600..400, y 0..1200)：木箱阵列 + 原木 ----------
    for i in range(3):
        for j in range(3):
            k.add('Wooden_Crate', -1300 + i * 300, 200 + j * 300, (i * 47 + j * 23) % 360)
    k.add('Wooden_Crate', -1300, 200, 12, 78)
    k.add('Log_Pile_1', -450, 1020, 0)
    k.add('Log_Pile_2', -180, 1020, 90)
    k.add('Log_Pile_3', 100, 1020, 0)
    k.add('Old_Planks_Plank_1', -1000, 1120, 20)
    k.add('Old_Planks_Plank_1', -1000, 1075, 25)
    k.add('Old_Planks_Plank_2', -900, 1120, 10)
    k.add('Old_Planks_Plank_4', -820, 1080, 15)
    k.add('Metal_Barrel', 250, 1120, 0)
    k.add('Metal_Bucket', 100, 400, 0)
    k.add('Wheel_Barrow', -1500, 700, 30)
    k.add('Cardboard_Box_1', -1500, 300, 0)
    k.add('Cardboard_Box_3', -1450, 250, 20)
    k.add('Pallet', -600, 1100, 0)
    k.add('Pallet_Broken', -400, 1120, 30)
    place_skm(CHEST + 'SKM_Box', 250, 300, 0, -20, label='MANOR_Chest_Store')

    # ---------- 卧室 (x 400..1600, y 0..1200)：起居 ----------
    k.add('Rocking_Chair', 800, 400, 0)
    k.add('Old_Sofa', 1230, 380, 180)
    k.add('Plastic_Chair', 700, 820, 90)
    k.add('Pumpkin_1', 1420, 1020, 0)
    k.add('Pumpkin_2', 1330, 1050, 0)
    k.add('Wooden_Crate', 1460, 260, 25)
    k.add('Cardboard_Box_2', 600, 1120, 0)
    k.add('Cardboard_Box_4', 700, 1130, -15)
    k.add('Metal_Bucket', 950, 1050, 0)
    k.add('Office_Chair', 1150, 1000, 200)
    save()
    unreal.log('MANOR: stage_props done, %d props, actors=%d' % (k.n, count()))
    return count()


def stage_outside():
    """室外：松树、藤蔓、岩石、篱笆、南瓜、旗帜。"""
    clear_prefix('MANOR_Out')
    o = _Kit('MANOR_Out')
    # 松树林
    for i, (x, y) in enumerate([(-2600, -2200), (-3200, -900), (-2900, 600), (-3400, 1900),
                                (-2400, 2500), (-600, 2700), (500, 2600), (1500, 2700),
                                (2500, 2500), (2900, 1200), (3000, -800), (2700, -2200),
                                (-1500, -2800), (600, -2900), (2200, -2900)]):
        o.add(['Pine_Tree', 'SM_Pine_Tree_01', 'SM_Pine_Tree_02', 'SM_Pine_Tree_03',
               'SM_Pine_Tree_04', 'SM_Pine_Tree_05'][i % 6], x, y, (i * 53) % 360,
              prop='Foliage')
    # 灌木 / 草丛 / 花
    for i, (x, y) in enumerate([(-1900, -1700), (-1950, -1100), (-2000, 900), (-1800, 1600),
                                (1900, -1500), (2000, -600), (1950, 1500), (1800, 1800),
                                (-900, 1800), (-200, 1900), (700, 1850), (1200, -1800)]):
        o.add(['Bush_1', 'Shrubs_1', 'Grass_Patch_1', 'Flower_Patch_1',
               'Grass_Patch_2', 'Grass_Patch_Long'][i % 6], x, y, (i * 71) % 360, prop='Foliage')
    # 藤蔓爬外墙
    for i, (x, y, yaw) in enumerate([(-1600, -900, 0), (-1600, -300, 0), (-1600, 700, 0),
                                     (1600, -900, 180), (1600, 300, 180),
                                     (-800, -1200, 90), (400, -1200, 90), (900, -1200, 90),
                                     (-800, 1200, 270), (200, 1200, 270)]):
        o.add(['Ivy_1', 'Ivy_2', 'Ivy_3', 'Ivy_4', 'Ivy_5', 'Ivy_6'][i % 6],
              x, y, yaw, prop='Foliage')
    # 岩石
    for i, (x, y) in enumerate([(-2100, -300), (-2050, 600), (2150, 400), (2100, -1000),
                                (-1200, 2200), (1100, 2100), (-2200, -2000), (2300, -2100)]):
        o.add(['Rock_1', 'River_Stone_1', 'River_Stone_2', 'River_Stone_3'][i % 4], x, y,
              (i * 41) % 360)
    # 篱笆环绕
    for i in range(9):
        o.add('Fence_Old_%d_2m' % (i % 3 + 1), -1600 + i * 400, -2000, 0)
    for i in range(6):
        o.add('Fence_Old_%d_2m' % (i % 3 + 1), -2000, -1800 + i * 400, 90)
    # 南瓜 / 杂物 / 旗帜
    o.add('Pumpkin_1', -900, -1450, 0)
    o.add('Pumpkin_2', -850, -1500, 0)
    o.add('Pumpkin_1', 750, -1500, 0)
    o.add('Metal_Bucket', -350, -1450, 0)
    o.add('Axe_1', -1250, -1400, 30)
    o.add('Axe_2', -450, -1380, -20)
    o.add('Mailbox_Pole', 1000, -1500, 0)
    o.add('Mailbox_Lid', 1000, -1500, 118, prop='Props')
    o.add('Flag', -1750, -1200, 0)
    o.add('Flag', 1750, 1200, 180)
    o.add('Old_Planks_Plywood_1', -1800, -800, 25)
    o.add('Old_Planks_Plank_1', -1850, -700, 10)
    o.add('Side_Shed', 1900, -500, -90)
    o.add('Wooden_Boat', -300, 2200, 30)
    save()
    unreal.log('MANOR: stage_outside done, %d props, actors=%d' % (o.n, count()))
    return count()


def count():
    return len(actor_sub().get_all_level_actors())


# ---------------------------------------------------------------- 截图

SHOT_DIR = None


def _shot_dir():
    global SHOT_DIR
    if SHOT_DIR is None:
        SHOT_DIR = unreal.Paths.convert_relative_path_to_full(
            unreal.Paths.project_saved_dir()) + 'ManorShots/'
        import os
        os.makedirs(SHOT_DIR, exist_ok=True)
    return SHOT_DIR


def shot(name, loc=None, rot=None, save=True):
    """截视口并存 PNG。loc/rot 为 None 时用视口当前相机。"""
    import json as _json
    import base64 as _b64
    args = {'captureTransform': None, 'annotations': None}
    if loc is not None:
        L = loc if isinstance(loc, unreal.Vector) else unreal.Vector(*loc)
        R = rot if rot is not None else unreal.Rotator()
        if not isinstance(R, unreal.Rotator):
            R = unreal.Rotator(roll=R.roll, pitch=R.pitch, yaw=R.yaw)
        args['captureTransform'] = {
            'location': {'x': L.x, 'y': L.y, 'z': L.z},
            'rotation': {'pitch': R.pitch, 'yaw': R.yaw, 'roll': R.roll},
            'scale': {'x': 1.0, 'y': 1.0, 'z': 1.0}}
    res = unreal.ToolsetRegistry.execute_tool(
        'EditorToolset.EditorAppToolset', 'CaptureViewport', _json.dumps(args))
    if not res.is_complete or res.error:
        return 'SHOT FAILED: ' + str(res.error)
    d = _json.loads(res.get_value_as_json_string())
    if isinstance(d, str):                 # 双重序列化
        d = _json.loads(d)
    img = (d.get('returnValue') or {}).get('image') or {}
    if not img.get('data'):
        return 'SHOT EMPTY: ' + _json.dumps(d)[:300]
    if not save:
        return 'OK bytes=' + str(len(img['data']))
    p = _shot_dir() + name + '.png'
    with open(p, 'wb') as f:
        f.write(_b64.b64decode(img['data']))
    return p


def camera(loc, pitch, yaw):
    """设置视口相机。"""
    L = loc if isinstance(loc, unreal.Vector) else unreal.Vector(*loc)
    unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).set_level_viewport_camera_info(
        L, unreal.Rotator(roll=0.0, pitch=pitch, yaw=yaw))


_PENDING = {}


def asset_shot(path, out=None):
    """
    渲染单个资产预览图（异步）。
    第一次调用发起，第二次调用同一 path 取结果。返回输出路径或 None(还没好)。
    """
    import json as _json
    import base64 as _b64
    import os
    if path not in _PENDING:
        _PENDING[path] = {}
        _PENDING[path]['handle'] = unreal.ToolsetRegistry.execute_tool(
            'EditorToolset.EditorAppToolset', 'CaptureAssetImage',
            _json.dumps({'assetPath': path}))
        return None
    h = _PENDING[path]['handle']
    if not h.is_complete:
        return None
    s = h.get_value_as_json_string()
    if not s:
        del _PENDING[path]
        return 'EMPTY'
    d = _json.loads(s)
    if isinstance(d, str):
        d = _json.loads(d)
    img = (d.get('returnValue') or {})
    img = img.get('image') if isinstance(img.get('image'), dict) else img
    del _PENDING[path]
    if not img.get('data'):
        return 'NO DATA: ' + str(d)[:200]
    if out is None:
        out = _shot_dir() + 'asset_' + path.split('/')[-1] + '.png'
    with open(out, 'wb') as f:
        f.write(_b64.b64decode(img['data']))
    return out


def lookat(eye, target, roll=0.0):
    """从 eye 看向 target，自动算 pitch/yaw 并设置视口相机。"""
    import math
    E = eye if isinstance(eye, unreal.Vector) else unreal.Vector(*eye)
    T = target if isinstance(target, unreal.Vector) else unreal.Vector(*target)
    d = T - E
    n = math.sqrt(d.x * d.x + d.y * d.y + d.z * d.z) or 1.0
    pitch = math.degrees(math.asin(max(-1.0, min(1.0, d.z / n))))
    yaw = math.degrees(math.atan2(d.x, d.y))
    camera(E, pitch, yaw)
    return pitch, yaw


# ---------------------------------------------------------------- 阶段 1: 关卡 + 环境

def stage_env():
    """新建关卡 L_Manor，铺地面与光照。幂等：先删旧关卡再重建。"""
    if unreal.EditorAssetLibrary.does_asset_exist(LEVEL_PATH):
        unreal.EditorAssetLibrary.delete_asset(LEVEL_PATH)
    ok = les().new_level(LEVEL_PATH)
    if LEVEL_PATH not in world_name():
        # delete_asset 之后 new_level 偶尔不切换，退回显式加载
        les().load_level(LEVEL_PATH)
    les().save_current_level()
    got = world_name()
    unreal.log('MANOR: new_level=%s world=%s' % (ok, got))
    if LEVEL_PATH not in got:
        raise RuntimeError('关卡未切换成功，当前世界: ' + got)
    clear_all()

    # --- 地面台座：用 snap_z 把 Diorama_Ground 顶面对齐到 z = 0（建筑基准面）
    g = place(UNIQUE + 'Diorama_Ground', 0, 0, z=0.0, label='MANOR_Ground', pivot='base')
    snap_z(g, 0.0, 'top')

    # --- 光照（对齐 Lr_Overview 的黄昏风格：pitch -50, yaw -10, int 3.0）
    sun = spawn_class(unreal.DirectionalLight, (0, 0, 800), 'MANOR_Sun',
                      unreal.Rotator(roll=0.0, pitch=-50.0, yaw=-10.0))
    c = sun.get_component_by_class(unreal.DirectionalLightComponent)
    c.set_editor_property('intensity', 3.0)
    c.set_editor_property('temperature', 4200.0)

    sky = spawn_class(unreal.SkyLight, (0, 0, 600), 'MANOR_SkyLight')
    c = sky.get_component_by_class(unreal.SkyLightComponent)
    c.set_editor_property('intensity', 1.0)
    c.set_editor_property('real_time_capture', True)

    spawn_class(unreal.SkyAtmosphere, (0, 0, 0), 'MANOR_SkyAtmosphere')
    spawn_class(unreal.VolumetricCloud, (0, 0, 0), 'MANOR_VolumetricCloud')

    fog = spawn_class(unreal.ExponentialHeightFog, (0, 0, 0), 'MANOR_Fog')
    fc = fog.get_component_by_class(unreal.ExponentialHeightFogComponent)
    fc.set_editor_property('fog_density', 0.012)
    fc.set_editor_property('fog_height_falloff', 0.18)
    fc.set_editor_property('start_distance', 20000.0)

    # 注意: 不放 SM_SkySphere —— 半径过大且 SkyAtmosphere 已负责天空
    save()
    unreal.log('MANOR: stage_env done, actors=%d' % count())
    return count()


# ---------------------------------------------------------------- 一键构建

def build_all():
    """完整生成冒险宅邸场景。可重复执行（幂等：重建 L_Manor）。"""
    unreal.ViewportService.set_view_mode('lit')
    unreal.ViewportService.set_exposure(True, -0.2)
    stages = [
        ('环境/光照', stage_env),
        ('主楼外壳', stage_shell),
        ('塔楼', stage_tower),
        ('门廊/大门/烟囱', stage_porch),
        ('屋顶', stage_roof),
        ('室内道具', stage_props),
        ('室外环境', stage_outside),
    ]
    for name, fn in stages:
        n = fn()
        unreal.log('  [MANOR] %-16s -> %d actors' % (name, n))
    save()
    lookat((5200, -6200, 3800), (0, 0, 500))
    unreal.log('MANOR: build_all complete, %d actors' % count())
    return count()


def report():
    """打印布局数值报告（视口截图通路不可用时的验证手段）。"""
    import collections
    g = collections.OrderedDict()
    lo, hi = 1e9, -1e9
    n_mesh = 0
    for a in actor_sub().get_all_level_actors():
        l = a.get_actor_label()
        if not l.startswith('MANOR'):
            continue
        r = world_z_range(a)
        if not r:
            continue
        n_mesh += 1
        lo, hi = min(lo, r[0]), max(hi, r[1])
        key = l.split('_')[1] if l.count('_') >= 1 else l
        g.setdefault(key, []).append(r)
    print('MANOR 布局报告  关卡=%s' % world_name())
    print('  网格 actor: %d   整体 z: %.0f .. %.0f cm (%.1f m)'
          % (n_mesh, lo, hi, hi / 100.0))
    print('  %-20s %5s  %s' % ('group', 'n', 'z range(cm)'))
    for k, v in g.items():
        print('  %-20s %5d  %.0f .. %.0f'
              % (k, len(v), min(x[0] for x in v), max(x[1] for x in v)))
    return n_mesh
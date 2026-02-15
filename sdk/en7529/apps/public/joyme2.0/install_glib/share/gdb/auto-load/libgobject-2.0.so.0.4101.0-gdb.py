import sys
import gdb

# Update module path.
dir_ = '/proj/mtk14508/workspace/branch_joyme3_0711/tclinux_phoenix/apps/public/joyme2.0/install_glib/share/glib-2.0/gdb'
if not dir_ in sys.path:
    sys.path.insert(0, dir_)

from gobject import register
register (gdb.current_objfile ())

$Id$

Geoclustering module enables building of geographical map displays that
scale to 100 000+ markers. This is achieved by displaying multiple nearby
objects as one marker, and doing so dynamically so that when zooming
out in map display, more and more objects are combined together, and number
of markers actually displayed at any one time remains manageable (<1000).

Module pipeline
---------------
Geoclustering module has no user-visible components, it is just a building
block in a whole pipeline of modules involved in creating a map display:

- Objects to be displayed (input data for Geoclustering) are Drupal
	content nodes having a CCK field for geographical coordinates.
	The CCK field is provided by "Geo Field" module, which is required by
	Geoclustering.

- Geoclustering maintains a "cluster tree" in a separate database table.
	Each cluster represents potentially many nodes located geographically
	close to each other. The tree has multiple levels, so that near tree
	leaves there are levels having thousands of clusters each representing
	only a few nodes, and at the other extreme the root level consists of
	a single node representing the whole Earth (all objects regardless of
	their location).

- Nodes or clusters are served out as some geoinfo protocol accepting BBOX
	arguments (f.ex. using the WFS module). Geoclustering provides a Views
	handler which dynamically selects appropriate tree level for each
	request and rewrites a Node view dynamically so that it returns
	clusters or nodes as appropriate. Tree level is selected based on
	maximum number of items to display (configured in standard settings
	for the view, typically <1000). Thus if requested BBOX contains
	less than this number of nodes, then nodes are returned; otherwise
	it returns clusters from most detailed tree level which has less than
	maximum number of items.

- A map display displays the map, requesting marker objects dynamically
	using the selected geoinfo protocol (f.ex. using OpenLayers module
	with BBOX Strategy and WFS protocol). When the user zooms out the map,
	larger and larger BBOXes are requested and thus less and less detailed
	tree levels are used.

- When input nodes are changed, Geoclustering automatically updates
	the tree in database accordingly, using hook_nodeapi().

Installation
------------
Geoclustering can be installed like any other Drupal module -- place it
in the modules directory for your site and enable it on the
`admin/build/modules` page. There are a number of dependencies as well
(e.g. Geo Field) which are pointed out to you when you enable the module.

Geo needs the following patches for Geoclustering to really work:
- http://drupal.org/files/issues/geo-filter-float.patch
- http://drupal.org/files/issues/geo-ewkb-parsing.patch
- http://drupal.org/files/issues/geo-883032.patch

If using WFS module, this also needs patching to support BBOX requests:
- http://drupal.org/files/issues/wfs-filter-changes.patch

Geoclustering trees configuration
---------------------------------
After installation, you should define a Geoclustering tree. Geoclustering
has no UI yet, so this is currently only possible programmatically.
Tree parameters are ctools exportables and you can define them via
hook_geoclustering_tree_params(). Example code in mymodule.module:

function mymodule_ctools_plugin_api($module, $api) {
  if ($module == "geoclustering_tree_params" &&
								$api == "geoclustering_tree_params") {
    return array('version' => 1);
  }
}

function mymodule_geoclustering_tree_params() {
  $tree=new stdClass();
  $tree->api_version = 1;
  $tree->name = 'mymodule_tree';
  $tree->description = 'Mymodule Geoclustering tree';
  $tree->maxlevel = 27;
  $tree->geofield_name = 'field_mycoordsfield';
  $tree->node_conditions = array('type' => 'mynodetype', 'status' => 1);
  $tree->summed_field_names = array('field_population', 'field_area');
  return array($tree->name => $tree);
}

Parameters are:
- name: Machine-readable unique tree name.
- description: Human-readable description of the tree. Optional.
- maxlevel: Number of cluster tree levels Geoclustering should build.
	Higher values mean more detailed clusters, but need more server
	horsepower. Default is 27 which means ca 2x2km clusters in the bottom
	of tree; increasing it by one halves the cluster area (to 1.4x1.4km),
	decreasing by one doubles the area (2.8x2.8km). Minimum is 0,
	which means a single cluster covering all Earth.
- node_conditions: PHP array of field_name => required_value mappings that
	specify requirements that a Node must meet in order to be included
	in this tree. field_name can be a {node} table field
	('type', 'status', ...) or a CCK field name (e.g. 'field_myfield').
	Multiple requirements are ANDed. Example: array('type' => 'mytype',
	'status' => 1, 'field_placemark_type' => 'school').
- geofield_name: CCK field name that stores Node coordinates for nodes
	that match node_conditions.
- summed_field_names: PHP array of CCK field names of numeric fields of
	input nodes. These fields are replicated from input nodes to clusters,
	so that values for each cluster are sums of values of its child nodes.
	Thus tree root node fields have sums of fields of all input nodes.
	Cluster fields have same types as node fields, thus for integer fields
	overflow could happen.

You can have multiple trees defined. When tree parameters are added or
modified, Geoclustering automatically recalculates the clustering tree
at the next Views request.

Views configuration
-------------------
Next you should define the view where Geoclustering does its dynamic
rewriting. This must be a Node view, and you should add
a "Geoclustering: Geoclustering level" argument to the view. In argument
settings dialog you must set which tree it operates on, and can also
tweak the "Default argument".

You should not add any filters to the view: Geoclustering adds tree
node_conditions as filters automatically, and specifying any other
filters would make Nodes view out of sync with clusters database which
has no such filters configured.

In the view you should also set "Items to display" (under "Basic settings")
to your desired max number of markers on map (usually 50..1000).
If set to unlimited, Geoclustering uses 1000 as default.

When selecting view Fields, you can add "Geo: Number of nodes" which
makes the numer of nodes in each cluster available via your geodata protocol.
For ordinary nodes this field is empty, allowing differentiating between
nodes and clusters in your map display styling.

If using WFS, you should also add a WFS display to view, with Style set
to "WFS Feed", "Path" set to your desired WFS requests URL, and
"Style options" set appropriately, for example:
	Map Data Sources: Lat/Lon Fields
	ID: Nid
	Longitude: Longitude (field_mycoordsfield)
	Latitude: Latitude (field_mycoordsfield)

Map display configuration
-------------------------
If using OpenLayers, you should probably implement
hook_openlayers_layer_types() to add a custom "Layer type" which uses
your geodata protocol (e.g. WFS) and OpenLayers.Strategy.BBOX. This needs
ca 100 lines PHP + 30 lines JavaScript; see http://drupal.org/node/629928
and docs/LAYER_TYPES.txt in OpenLayers module.

Maintainers
-----------
- ahtih (Ahti Heinla)

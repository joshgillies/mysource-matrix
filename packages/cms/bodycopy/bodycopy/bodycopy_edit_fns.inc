<?php

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
require_once SQ_FUDGE_PATH.'/general/general.inc';


/**
* Print a bodycopy style icon
*
* This is normally used to print the bodycopy icons in the 
* for bodycopy, table, row and cell functions
*
* @param   string $href		The HREF that this icons points to
* @param   string $heading	The heading for the tooltip
* @param   string $desc		The main body of the tooltip
* @param   string $icon		The filename of the icon to display (must be a GIF file)
* @param   string $extra	Any extras you want to put at the end of the IMG tag
* @param   string $width	The width of the icon
* @param   string $height	The height of the icon
* @access  public
* @returns void
*/
function print_bodycopy_icon($href, $heading, $desc, $icon, $extra='', $width='20', $height='20'){
	?>
	<a href="<?php echo $href?>" onmouseover="javascript: show_tooltip(event, '<?php echo addslashes(htmlspecialchars($heading))?>', '<?php echo addslashes(htmlspecialchars($desc))?>', null, 'bodycopyToolTipDiv'); return true;" onmouseout="javascript:hide_tooltip(); return true;" ><img src="<?php echo sq_web_path('data')?>/bodycopy/files/images/icons/<?php echo $icon?>.gif" width="<?php echo $width?>" height="<?php echo $height?>" border="0" <?php echo $extra?>></a><?php
}// end print_bodycopy_icon()


/**
* Bodycopy_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage cms
*/
class Bodycopy_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bodycopy_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
	}


	/**
	* Paints this bodycopy as HTML
	*
	* @param object Bodycopy	$asset	the bodycopy
	*
	* @return void
	* @access public
	*/
	function paint(&$asset)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		// get all the tables linked to this bodycopy
		$tables = $asset->getLinks(SQ_LINK_UNITE, 'bodycopy_table');
		
		// paint each table
		$i = 0;
		foreach ($tables as $table_data) {
			$table = &$am->getAsset($table_data['minorid'], $table_data['type_code']);
			$table_edit = $table->getEditFns();
			$table_edit->paint($table);
		}

	}// end paint()


	/**
	* Paint the interface for creating tables in the bodycopy
	*
	* @param object Bodycopy			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopy(&$asset, &$o, $prefix)
	{
		global $SQ_FUDGE_PATH;

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		// get all the tables linked to this bodycopy
		$tables = $asset->getLinks(SQ_LINK_UNITE, 'bodycopy_table');

		if (!$asset->writeAccess()) {
			foreach ($tables as $table_data) {
				$table = &$am->getAsset($table_data['minorid'], $table_data['type_code']);
				$table_edit = $table->getEditFns();
				$table_edit->paintTable($table, $o, $prefix);
			}
			return true;
		}
		
		// the initial width and height to have the layer display created at;
		$init_layer_width  = 500;
		$init_layer_height = 500;

		// all available content types for the table cells
		$content_types = array();
		$types = $am->getAssetTypeHierarchy('content_type');
		foreach ($types as $type => $type_data) $content_types[$type] = array('name' => $type_data['name']);

		?>
		<input type="hidden" name="bodycopy_action" value="">
		<input type="hidden" name="bodycopy_name"   value="">
		<input type="hidden" name="bodycopy_data"   value="">

		<script language="JavaScript" src="<?php echo sq_web_path('data').'/bodycopy/files/js/bodycopy_edit_tables.js'?>"></script>
		<script language="JavaScript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js'?>"	></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/detect.js';?>"					></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/layer_handler.js';?>"				></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/general.js';?>"					></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/debug.js';?>"						></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/tooltips.js';?>"					></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js';?>"			></script>
		
		<script language="JavaScript">
			// this is an array of data that each element (tables, rows, and cells)
			// can place data for use in editing
			var bodycopy_current_data = new Object();
			var bodycopy_saved        = new Object();

			// this is an object that gets set everytime something is getting edited 
			// (as apposed to inserted or deleted) so that the pop-ups can get the info once
			// they have loaded themselves
			var bodycopy_current_edit = new Object();
			bodycopy_current_edit["data"] = null;
			bodycopy_current_edit["bodycopy_name"] = null;

			var bodycopy_initialised = false;	// true once init() has been run
			var bodycopy_popup   = null;		// pointer to the popup Layer_Handler Object
			var bodycopy_popup_visible = false;	// boolean indicating whether the popup is visisble ( not access directly )
			var bodycopy_nested_doc = null;		// pointer to the Netscape Layer or the IE Iframe

			// initialise the popup
			function bodycopy_init() {
				bodycopy_popup   = new Layer_Handler("bodycopyPopupDiv", 0,<?php echo $init_layer_width?>,<?php echo $init_layer_height?>,0);
				set_bodycopy_nested_doc();
				bodycopy_hide_popup();
				bodycopy_initialised = true;
				bodycopy_otheronload();
			}

			var bodycopy_otheronload = (window.onload) ? window.onload :  new Function;
			window.onload = bodycopy_init;

			// generic function used everywhere to send the form
			function bodycopy_submit(bodycopy_action, bodycopy_name, bodycopy_data) {
				var form = document.main_form;

				form.bodycopy_action.value = bodycopy_action;
				form.bodycopy_name.value   = bodycopy_name;
				
				// pack up the passed object
				form.bodycopy_data.value = var_serialise(bodycopy_data);

				// need to call the onsubmit event explicitly or it will
				// not get called when we do a form.submit()
				form.onsubmit();
				form.submit();
			}// end bodycopy_submit()

			function serialise_table(bodycopy_name, bodycopy_data, tableid, rowid, cellid) {
				var form = document.main_form;
				if (!bodycopy_saved[bodycopy_name] && bodycopy_name != null) bodycopy_saved[bodycopy_name] = new Object();

				if (tableid != null) {
					if (!bodycopy_saved[bodycopy_name][tableid]) {
						bodycopy_saved[bodycopy_name][tableid] = new Object();
					}

					if (rowid != null) {
						if (!bodycopy_saved[bodycopy_name][tableid]['rows']) {
							bodycopy_saved[bodycopy_name][tableid]['rows'] = new Object();
						}
						if (!bodycopy_saved[bodycopy_name][tableid]['rows'][rowid] && rowid != null) {
							bodycopy_saved[bodycopy_name][tableid]['rows'][rowid] = new Object();
						} 
						if (cellid != null) {
							if (!bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['cells']) { 
								bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['cells'] = new Object();
							}
							bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['cells'][cellid] = bodycopy_data;
						} else {
							// no cellid means that we are setting row attributes
							bodycopy_saved[bodycopy_name][tableid]['rows'][rowid]['attributes'] = bodycopy_data['attributes'];
						}// end if cellid

					} else {
						// no rowid means that we are settign the table attributes
						bodycopy_saved[bodycopy_name][tableid]['attributes'] = bodycopy_data['attributes'];
					}//end if rowid

				}// end if tableid

				form.elements['bodycopy_saved[' + bodycopy_name + '][' + tableid + ']'].value = var_serialise(bodycopy_saved[bodycopy_name][tableid]);
			}// end serialise_table()

			function set_bodycopy_nested_doc() {
				if (is_ie4up) {
					bodycopy_nested_doc = bodycopyFrame;
				} else if (is_dom) {
					bodycopy_nested_doc = document.getElementById("bodycopyFrame");
				} else {
					bodycopy_nested_doc = bodycopy_popup.layer;
				}//end if
			}

			function get_bodycopy_popup_visibilty() {
				return bodycopy_popup_visible;
			}

			function bodycopy_show_popup(file, width, height) {
				if (!bodycopy_initialised) {
					if (confirm('You were too quick off the mark, the page is not loaded yet. Do you want to reload the page?')) {
						document.edit.action.value='';
						document.edit.submit()
					}
					return;
				}

				var w = (width  != null) ? width  : 500;
				var h = (height != null) ? height : 400;
				var border = 15;
				var top_offset = 20;

				var page_w = (is_nav4 || is_gecko) ? w - 17 : w;
				var page_h = (is_nav4 || is_gecko) ? h - 17 : h;

				file  = '<?php echo $asset->getBackendHref('show_popup');?>' + '&popup_file=' + file;
				file += '&page_width=' + page_w;
				file += '&page_height=' + page_h;
				file += '&body_extra=';
				file += '&browser=' + ((is_dom) ? "dom" : ((is_ie4up) ? "ie" : "ns"));

				bodycopy_popup.w = w;
				bodycopy_popup.h = h;
				bodycopy_popup.clip(null, w, h, null);
				bodycopy_popup_visible = true;

				if (is_ie4up) {
					// %!%#, frameElement is IE 5.5 only
					bodycopy_nested_doc.width  = w - border;
					bodycopy_nested_doc.height = h - border;
					bodycopy_nested_doc.location = file;
				} else if (is_dom) {
					bodycopy_nested_doc.width  = w - border;
					bodycopy_nested_doc.height = h - border;
					bodycopy_nested_doc.src = file;
				} else {
					bodycopy_nested_doc.clip.right     = w - border;
					bodycopy_nested_doc.clip.width     = w - border;
					bodycopy_nested_doc.clip.height    = h - border;
					bodycopy_nested_doc.clip.bottom    = h - border;
					bodycopy_nested_doc.load(file, w - 5);
				}

				// how far down the page we want to display this popup
				var scroll_top  = ((is_ie4up) ? document.body.scrollTop  : self.pageYOffset);
				bodycopy_popup.move(null, scroll_top + top_offset);
				bodycopy_popup.show();
			}// end bodycopy_show_popup()

			function bodycopy_hide_popup() {
				if (!bodycopy_initialised) { return; }
				bodycopy_show_popup("blank.php");
				bodycopy_popup_visible = false;
				bodycopy_popup.hide();
			}// end bodycopy_hide_popup()

			// get the available cell types 
			var BODYCOPY_AVAILABLE_CONTENT_TYPES = null;
			function get_bodycopy_available_content_types() {
				if (BODYCOPY_AVAILABLE_CONTENT_TYPES == null) {
					BODYCOPY_AVAILABLE_CONTENT_TYPES = var_unserialise('<?php echo var_serialise($content_types, true)?>');
				}
				return BODYCOPY_AVAILABLE_CONTENT_TYPES;
			}

			// general fn that the generic include fns can use to get data to use
			function get_bodycopy_current_data(bodycopy_name, tableid, rowid, cellid) {
				if (tableid == null) tableid = -1;
				if (rowid   == null) rowid   = -1;
				if (cellid  == null) cellid  = -1;
				return bodycopy_current_data[bodycopy_name].get_data(tableid, rowid, cellid);
			}

			function bodycopy_data_exists(args) {
				var str = 'bodycopy_saved';
				for(var i = 0; i < args.length; i++) {
					switch(typeof(args[i])) {
						case "number" :
							str += '[' + args[i] + ']';
						break;
						default :
							str += '["' + args[i] + '"]';
					}
					eval('var exists = (' + str + ') ? true : false;');
					if (!exists) return false;
				}
				return true;
			}

			function bodycopy_chgColor(id, colour) {
				if (is_dom) {
					var chgcell
					if (!colour) { colour = '559AE7'; }
					chgcell = "document.getElementById('"+ id + "').style.backgroundColor = '#"+ colour +"'";
					eval(chgcell);
				}
			}
		</script>

		<style type="text/css">
			#bodycopyPopupDiv {
				color: #fffff;
				position:absolute; 
				left:100; 
				top:120; 
				width:<?=$init_layer_width?>; 
				height:<?=$init_layer_height?>; 
				clip:rect(0,600,600,0);
				visibility: hidden;
			}
			#bodycopyToolTipDiv {
				position:absolute; 
				width:  1px; 
				height: 1px; 
				visibility: hidden;
			}
		</style>

		<!-- Begin Tooltip -->
		<div id="bodycopyToolTipDiv" style="z-index:1000"></div>
		<!-- End Tooltip -->

		<!-- Begin Popup -->
		<div id="bodycopyPopupDiv" style="z-index:1001">
			<iframe name="bodycopyFrame" id="bodycopyFrame" scrolling="no" width="<?php echo $init_layer_width?>" height="<?php echo $init_layer_height?>" marginwidth=0 marginheight=0 frameborder="no"></iframe>
		</div>
		<!-- End Popup -->

		<script language="JavaScript">
			bodycopy_current_data["<?php echo $prefix?>"] = new Object();
			function get_bodycopy_<?php echo $prefix?>_current_data(tableid, rowid, cellid) {
	
				retVal = new Object();

				switch (tableid) {
			<?php

			// if we have tables get them to print the js data they need
			if (count($tables)) {
				$i = 0;
				foreach ($tables as $table_data) {
					$table = &$am->getAsset($table_data['minorid'], $table_data['type_code']);
					$table_edit = $table->getEditFns();
					$table_edit->paintBackendJsData($table, $prefix, $i);
				}
			}
			
			?> 
					default :
						retVal["num_tables"] = <?php echo count($tables)?>;

				}// end switch tableid


				// check if this piece of data has been changed before
				// if it has, return the changed version, not the original
				if (tableid != -1) {
					if (rowid != -1) {
						if (cellid != -1) {
							if (bodycopy_saved && 
									bodycopy_saved["<?php echo $prefix?>"] && 
									bodycopy_saved["<?php echo $prefix?>"][tableid] && 
									bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"] &&
									bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid] &&
									bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"] &&
									bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"][cellid]) {
								for (var key in bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"][cellid]) {
									retVal[key] = var_serialise(bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["cells"][cellid][key]); 
								}
							}
							return retVal;
						} else { 
							// editing row properties
							if (bodycopy_data_exists(new Array('<?php echo $prefix?>', tableid, 'rows', rowid, 'attributes'))) {
								retVal["attributes"] = var_serialise(bodycopy_saved["<?php echo $prefix?>"][tableid]["rows"][rowid]["attributes"]);
								return retVal;
							}
						}
					} else {
						// editing table properties
						if (bodycopy_data_exists(new Array('<?php echo $prefix?>', tableid, 'attributes'))) {
							retVal ["attributes"] = var_serialise(bodycopy_saved["<?php echo $prefix?>"][tableid]["attributes"]);
							return retVal;
						}
					}
				}

				return retVal;
			}// end get_bodycopy_<?php echo $prefix?>_current_data()

			// set reference so generic fn can be called above
			bodycopy_current_data["<?php echo $prefix?>"].get_data    = get_bodycopy_<?php echo $prefix?>_current_data;
		</script>

		<table border="0" cellspacing="0" cellpadding="0" width="100%" class="bodytext">
		
		<?php

		// for each table, print it's editing interface
		$i = 0;
		foreach ($tables as $table_data) {
			$table = &$am->getAsset($table_data['minorid'], $table_data['type_code']);
			$table_edit = $table->getEditFns();
		?> 
			<tr>
				<td>
					<input type="hidden" name="bodycopy_saved[<?php echo $prefix?>][<?php echo $table->id?>]" value="">
					<table border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td>
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table('$prefix', $i, true)", 'Insert Table', '', 'insert_table');
							?>
							</td>
							<td>
								&nbsp;&nbsp;Create New Table&nbsp;&nbsp;
							</td>
						</tr>
					</table>
				</td> 
			</tr>
			<tr>
				<td>&nbsp;</td>
			</tr>
			<tr>
				<td><?php $table_edit->paintTable($table, $o, $prefix); ?></td>
			</tr>
			<tr>
				<td>&nbsp;</td>
			</tr>
		<?php
			$i++;
		}// end foreach table
		?>

			<tr>
				<td>
					<table border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td>
							<?php
							print_bodycopy_icon("Javascript: bodycopy_insert_table('$prefix', $i, false)", 'Insert Table', '', 'insert_table');
							?>
							</td>
							<td>
								&nbsp;&nbsp;Create New Table&nbsp;&nbsp;
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<?php

		return true;

	}// end paintBodycopy()


	/**
	* Processes the interface for creating tables in the bodycopy
	* This function processes some actions (such as insert_table) but
	* lets ndividual tables handle other actions themselves
	*
	* @param object Asset				$asset	the asset to which we belong
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processBodycopy(&$asset, &$o, $prefix)
	{
		global $bodycopy_action, $bodycopy_name, $bodycopy_data, $bodycopy_saved;
		global $_BODYCOPY;

		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$updated = false;
		
		// do some thing for the first time we print a bodycopy on this page
		if (empty($_BODYCOPY['update_initialised'])) {
			$bodycopy_action = (empty($_POST['bodycopy_action'])) ? ''      : gpc_stripslashes($_POST['bodycopy_action']);
			$bodycopy_name   = (empty($_POST['bodycopy_name']))   ? ''      : gpc_stripslashes($_POST['bodycopy_name']);
			$bodycopy_data   = (empty($_POST['bodycopy_data']))   ? Array() : gpc_stripslashes($_POST['bodycopy_data']);
			$bodycopy_saved  = (empty($_POST['bodycopy_saved']))  ? Array() : gpc_stripslashes($_POST['bodycopy_saved']);
	
			if (empty($bodycopy_data) && empty($bodycopy_saved)) return false;

			// if there is some bodycopy data deserialise it into the arrays
			// so that everything else can use it
			if ($bodycopy_data) $bodycopy_data = var_unserialise($bodycopy_data);

			for (reset($bodycopy_saved); NULL !== ($k = key($bodycopy_saved)); next($bodycopy_saved)) {
				$tables = &$bodycopy_saved[$k];
				for (reset($tables); NULL !== ($tableid = key($tables)); next($tables)) {
					$tables[$tableid] = var_unserialise($tables[$tableid]);
				}
			}
			unset($tables);

			$_BODYCOPY['update_initialised'] = true;
		}

		// get all the tables linked to this bodycopy
		$tables = $asset->getLinks(SQ_LINK_UNITE, 'bodycopy_table');

		foreach ($tables as $table_data) {
			$table = &$am->getAsset($table_data['minorid'], $table_data['type_code']);
			$table_edit = $table->getEditFns();
			if ($table_edit->processTable($table, $o, $prefix)) $updated = true;
		}

		// only update if we are the bodycopy that submitted the form
		if ($bodycopy_name == $prefix || isset($bodycopy_saved[$prefix])) {

			// update the tables
			if ($bodycopy_name == $prefix) {
				switch($bodycopy_action) {
					
					case 'insert_table' :
							if ($this->insertTable($asset, $bodycopy_data['tableid'],	$bodycopy_data['num_rows'], 
																						$bodycopy_data['num_cols'], 
																						$bodycopy_data['before'],
																						$bodycopy_data['attributes']
							)) $updated = true;
						break;

					case 'delete_table' :
							if ($this->deleteTable($asset, $bodycopy_data['tableid'])) $updated = true;
						break;
				}
			}
		}

		return $updated;

	}//end processBodycopy()


	/**
	* Create a new bodycopy_table asset and attach it to this bodycopy
	*
	* @param object Bodycopy	&$asset		the bodycopy
	* @param int				$tableid	insert this table before or after this tableid
	* @param int				$num_rows	the number of rows the new table should have
	* @param int				$num_cols	the number of cells the new table should have
	* @param boolean			$before		insert this table before the passed tableid
	* @param array				$attributes	an array of attributes to apply to the new table
	*
	* @return boolean
	* @access public
	*/
	function insertTable(&$asset, $tableid, $num_rows=1, $num_cols=1, $before=true, $attributes=Array())
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// get all the tables linked to this bodycopy
		$tables = $asset->getLinks(SQ_LINK_UNITE, 'bodycopy_table');
		$num_tables = count($tables);

		$tableid = intval($tableid);

		// if the table is greater than the number of tables we have, set it to the last one
		if ($tableid > $num_tables - 1) $tableid = $num_tables - 1;
		else if ($tableid < 0) $tableid = 0;

		// if they want to insert after the passed tableid what 
		// they really mean is to insert before the next tableid
		if (!$before) $tableid++;

		// now add the table
		$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_UNITE, 'value' => '', 'sort_order' => $tableid);
		$am->includeAsset('bodycopy_table');
		$new_table = new Bodycopy_Table();
		$new_table->setAttrValue('name', 'Table '.$tableid);
		$new_table->setAttrValue('attributes', $attributes);
		if (!$new_table->create($copy_link, $num_rows, $num_cols)) return false;

	}//end insertTable()


	/**
	* Unlink a table from this bodycopy (effectivly deleting it from this bodycopy)
	*
	* @param object Asset	&$asset		the bodycopy
	* @param int			$tableid	the ID of the table to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteTable(&$asset, $tableid)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// get all the tables linked to this bodycopy
		$tables = $asset->getLinks(SQ_LINK_UNITE, 'bodycopy_table');
		foreach ($tables as $table_data) {
			if ($table_data['minorid'] == $tableid) {
				// found the table to delete
				if (!$asset->deleteLink($table_data['linkid'])) return false;
				return true;
			}
		}

		return false;
	}//end deleteTable()


}//end class
?>
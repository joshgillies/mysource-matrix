<?php
/**
* Copyright (c) 2003 - Squiz Pty Ltd
*
* $Id: bodycopy_edit_fns.inc,v 1.41 2003/11/05 01:35:17 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';


/**
* Print a bodycopy style icon
*
* @param string $href		The HREF that this icons points to
* @param string $heading	The heading for the tooltip
* @param string $desc		The main body of the tooltip
* @param string $icon		The filename of the icon to display (must be a GIF file)
* @param string $extra		Any extras you want to put at the end of the IMG tag
* @param string $width		The width of the icon
* @param string $height		The height of the icon
*
* @access  public
* @returns void
*/
function print_bodycopy_icon($href, $heading, $desc, $icon, $extra='', $width='16', $height='16')
{
	?>
	<a href="<?php echo $href?>" onmouseover="javascript: show_tooltip(event, '<?php echo addslashes(htmlspecialchars($heading))?>', '<?php echo addslashes(htmlspecialchars($desc))?>', null, 'bodycopyToolTipDiv'); return true;" onmouseout="javascript:hide_tooltip(); return true;" >
	<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo sq_web_path('data')?>/asset_types/bodycopy/images/icons/<?php echo $icon?>.png", "<?php echo $width?>", "<?php echo $height?>", "");</script>
	</a>
	<?php

}// end print_bodycopy_icon()


/**
* Bodycopy_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
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

		unset($this->static_screens['permissions']);
		unset($this->static_screens['workflow']);
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['linking']);
		unset($this->static_screens['settings']);

	}//end constructor


	/**
	* Paint the interface for creating containers in the bodycopy
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
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		
		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);

		if (!$asset->writeAccess('content')) {
			foreach ($containers as $container_data) {
				$container = &$am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
				if ($container->useSystemVersion()) {
					$container->printBody();
				} else {
					$container_edit = $container->getEditFns();
					$container_edit->paintContainer($container, $o, $prefix);
				}
			}
			return true;
		}
		
		// the initial width and height to have the layer display created at;
		$init_layer_width  = 500;
		$init_layer_height = 500;

		// all available content types
		$content_types = array();
		$types = $am->getAssetTypeHierarchy('content_type');
		foreach ($types as $type => $type_data) $content_types[$type] = array('name' => $type_data['name']);

		?>
		<input type="hidden" name="bodycopy_action" value="">
		<input type="hidden" name="bodycopy_name"   value="">
		<input type="hidden" name="bodycopy_data"   value="">

		<script language="JavaScript" src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js'?>"	></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/detect.js';?>"					></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/layer_handler.js';?>"				></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/general.js';?>"					></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/debug.js';?>"						></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/js/tooltips.js';?>"					></script>
		<script language="JavaScript" src="<?php echo sq_web_path('lib').'/html_form/html_form.js';?>"			></script>
		
		<script language="JavaScript" type="text/javascript">
			// this is an array of data that each element (divs, tables, rows, and cells)
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
			var bodycopy_popup = null;			// pointer to the popup Layer_Handler Object
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

			function bodycopy_insert_container(bodycopy_name, containerid, before) {
				var form = document.main_form;
				var container_type = form_element_value(form.<?php echo $prefix; ?>_insert_container_type);
				eval('bodycopy_insert_' + container_type + '("' + bodycopy_name + '", ' + containerid + ', ' + before + ');');
			}

			// This function switches the view between wysiwyg and
			// page content for a wysiwyg editing field on the page
			var initialisedEditors = new Array();
			function switchEditingMode(contentDivID, editDivID, editor) {
				var contentDiv = document.getElementById(contentDivID); // div with page contents
				var editDiv = document.getElementById(editDivID);       // div with wysiwyg

				if (editDiv.style.display == "none") { // the edit div is hidden
					var setDesignMode = true;
					
					// initilise the wysiwg if this is the first time
					// it is being shown - skip this otherwise
					if (initialisedEditors[editor._uniqueID] == null) {
						initialisedEditors[editor._uniqueID] = true;
						editor.generate();
						//editor.updateToolbar(false);
						setDesignMode = false;
					} else if (editor._initialised != true) {
						return;
					}

					editDiv.style.display = ""; // show the wysiwyg

					// if we are using an iframe for this editor, we resize it
					// and set its designMode property if we need to
					if (editor._iframe) {
						editor._iframe.style.width = "100%";
						if (setDesignMode && HTMLArea.is_gecko) { editor._iframe.contentWindow.document.designMode = "on"; }
						var iframeHeight = contentDiv.offsetHeight - editor._toolbar.offsetHeight - editor._statusBar.offsetHeight;
						if (iframeHeight < 0) { iframeHeight = 400; }
						editor._iframe.style.height = iframeHeight + 'px';
					}
					contentDiv.style.display = "none"; // hide the contents
				} else if (editor._initialised == true) {
					// the content div is hidden and the
					// wysiwyg editor has been initialised
					contentDiv.innerHTML = editor.getHTML();
					contentDiv.style.display = "";  // show the contents
					editDiv.style.display = "none"; // hide the wysiwyg

					if (editor._iframe) {
						var editCell = document.getElementById(editor._uniqueID + "_cell");
						if (editCell) { editCell.style.height = "100%"; }
					}
				}
			}

			bodycopy_current_data["<?php echo $prefix?>"] = new Object();
		</script>

		<?php 
		// print JS functions for each of the container types we have installed
		$container_types = $am->getAssetTypeHierarchy('bodycopy_container');
		foreach ($container_types as $type => $type_data) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
			$type_asset = new $type();
			$edit_fns = $type_asset->getEditFns();
			$edit_fns->paintGenericBackend($asset, $o, $prefix);
		}

		$container_type_options = Array();
		$default = '';
		foreach ($container_types as $type => $type_data) {
			$code = strtolower(substr($type,9));
			$container_type_options[$code] = ucfirst(substr($type,9));
			if (empty($default)) $default = $code;
		}
		echo hidden_field($prefix.'_insert_container_type', $default);
		?>

		<style type="text/css">
			#bodycopyPopupDiv {
				color: #fffff;
				position:absolute; 
				left:100; 
				top:120; 
				width:<?php echo $init_layer_width?>; 
				height:<?php echo $init_layer_height?>; 
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

		<table border="0" cellspacing="0" cellpadding="0" width="100%" class="bodytext">
		
		<?php

		// for each container, print it's editing interface
		$i = 0;
		foreach ($containers as $container_data) {
			$container = &$am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			$container_edit = $container->getEditFns();
		?> 
			<tr>
				<td>
					<input type="hidden" name="bodycopy_saved[<?php echo $prefix?>][<?php echo $container->id?>]" value="">
					<table border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td>
							<?php
							print_bodycopy_icon("Javascript: document.main_form.{$prefix}_insert_container_type.value = document.main_form.{$prefix}_insert_container_type_before_{$i}.value; bodycopy_insert_container('$prefix', $i, true)", 'Insert Container', 'Select the type of container you want to create before clicking here', 'create_container');
							?>
							</td>
							<td class="sq-backend-data" valign="top">
								<?php
								echo '&nbsp;&nbsp;Create New ';
								echo combo_box($prefix.'_insert_container_type_before_'.$i, $container_type_options, false, '');
								?>
							</td>
						</tr>
					</table>
				</td> 
			</tr>
			<tr>
				<td>&nbsp;</td>
			</tr>
			<tr>
				<td><?php $container_edit->paintContainer($container, $o, $prefix); ?></td>
			</tr>
			<tr>
				<td>&nbsp;</td>
			</tr>
		<?php
			$i++;
		}// end foreach container
		?>

			<tr>
				<td>
					<table border="0" cellspacing="0" cellpadding="0">
						<tr>
							<td>
							<?php
							print_bodycopy_icon("Javascript: document.main_form.{$prefix}_insert_container_type.value = document.main_form.{$prefix}_insert_container_type_after_{$i}.value; bodycopy_insert_container('$prefix', $i, false)", 'Insert Container', 'Select the type of container you want to create before clicking here', 'create_container');
							?>
							</td>
							<td class="sq-backend-data" valign="top">
								<?php
								echo '&nbsp;&nbsp;Create New ';
								echo combo_box($prefix.'_insert_container_type_after_'.$i, $container_type_options, false, '');
								?>
							</td>
						</tr>
					</table>
				</td>
			</tr>
		</table>
		<?php

		return true;

	}//end paintBodycopy()


	/**
	* Processes the interface for creating containers in the bodycopy
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
		$am = &$GLOBALS['SQ_SYSTEM']->am;
		$updated = false;
		
		// do some thing for the first time we process a bodycopy on this page
		if (empty($this->_tmp['update_initialised'])) {
			$bc_action = (empty($_POST['bodycopy_action'])) ? ''      : $_POST['bodycopy_action'];
			$bc_name   = (empty($_POST['bodycopy_name']))   ? ''      : $_POST['bodycopy_name'];
			$bc_data   = (empty($_POST['bodycopy_data']))   ? Array() : $_POST['bodycopy_data'];
			$bc_saved  = (empty($_POST['bodycopy_saved']))  ? Array() : $_POST['bodycopy_saved'];
	
			if (empty($bc_data) && empty($bc_saved)) return false;

			// if there is some bodycopy data, deserialise it into the arrays
			// so that everything else can use it
			if ($bc_data) $bc_data = var_unserialise($bc_data);

			for (reset($bc_saved); NULL !== ($k = key($bc_saved)); next($bc_saved)) {
				$containers = &$bc_saved[$k];
				for (reset($containers); NULL !== ($containersid = key($containers)); next($containers)) {
					$containers[$containersid] = var_unserialise($containers[$containersid]);
				}
			}
			unset($containers);

			$this->_tmp['update_initialised'] = true;
		}

		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);

		// process each of the contianers
		foreach ($containers as $container_data) {
			$container = &$am->getAsset($container_data['minorid'], $container_data['minor_type_code']);
			$container_edit = $container->getEditFns();
			if ($container_edit->processContainer($container, $o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved)) {
				if ($container->saveAttributes()) $updated = true;
			}
		}

		// only update if we are the bodycopy that submitted the form
		if ($bc_name == $prefix || isset($bc_saved[$prefix])) {
			// perform some bodycopy level actions
			if ($bc_name == $prefix) {
				switch($bc_action) {
					case 'insert_container' :
						if ($this->insertContainer($asset, $bc_data['container_type'], $bc_data['containerid'], $bc_data)) $updated = true;
					break;

					case 'delete_container' :
						if ($this->deleteContainer($asset, $bc_data['containerid'])) $updated = true;
					break;
				}//end switch
			}
		}

		return $updated;

	}//end processBodycopy()


	/**
	* Insert a new container into this bodycopy
	*
	* @param object Asset	&$asset			the bodycopy
	* @param string			$type			the type of the container (table, div etc)
	* @param int			$containerid	the ID of the container to insert before/after
	* @param array			$bc_data		the array of bodycopy data that got posted
	*
	* @return boolean
	* @access public
	*/
	function insertContainer(&$asset, $type, $containerid, $bc_data)
	{
		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);
		$num_containers = count($containers);

		$containerid = intval($containerid);

		// if the containerid is greater than the number of containers we have, set it to the last one
		if ($containerid > $num_containers - 1) $containerid = $num_containers - 1;
		else if ($containerid < 0) $divid = 0;

		// if they want to insert after the passed containerid what
		// they really mean is to insert before the next containerid
		if (!$bc_data['before']) $containerid++;

		// now add the container
		$copy_link = Array('asset' => &$asset, 'link_type' => SQ_LINK_TYPE_2, 'value' => '', 'sort_order' => $containerid, 'dependant' => 1);

		$asset_type = 'bodycopy_'.$type;
		$GLOBALS['SQ_SYSTEM']->am->includeAsset($asset_type);
		$new_container = new $asset_type();
		$new_container->setAttrValue('name', 'New '.ucfirst($type));
		$new_container->setAttrValue('attributes', $bc_data['attributes']);
		$retval = (bool) $new_container->create($copy_link, $bc_data);

		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($new_container);
		return $retval;

	}//end insertContainer()


	/**
	* Unlink a container from this bodycopy (effectivly deleting it from this bodycopy)
	*
	* @param object Asset	&$asset			the bodycopy
	* @param int			$containerid	the ID of the container to delete
	*
	* @return boolean
	* @access public
	*/
	function deleteContainer(&$asset, $containerid)
	{
		$am = &$GLOBALS['SQ_SYSTEM']->am;

		// get all the containers linked to this bodycopy
		$containers = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_TYPE_2, 'bodycopy_container', false);
		foreach ($containers as $container_data) {
			if ($container_data['minorid'] == $containerid) {
				// found the container to delete
				return $asset->deleteLink($container_data['linkid']);
			}
		}

		return false;

	}//end deleteContainer()

}//end class

?>
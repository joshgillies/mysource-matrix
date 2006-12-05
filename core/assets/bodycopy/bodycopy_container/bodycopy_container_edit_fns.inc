<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: bodycopy_container_edit_fns.inc,v 1.47 2006/12/05 05:39:48 emcdonald Exp $
*
*/

require_once SQ_INCLUDE_PATH.'/asset_edit/asset_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/var_serialise/var_serialise.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';
require_once SQ_CORE_PACKAGE_PATH.'/bodycopy/bodycopy/bodycopy_edit_fns.inc';

/**
* Bodycopy_Container_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.47 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Bodycopy_Container_Edit_Fns extends Asset_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bodycopy_Container_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		unset($this->static_screens['metadataSchemas']);
		unset($this->static_screens['metadata']);
		unset($this->static_screens['settings']);

	}//end constructor


	/**
	* Set an attribute value in the attributes var
	*
	* Internal checking of these attributes has to go here as they will not be checked by the system
	*
	* @param array	&$attributes	a reference to the attributes var of the BodyCopy_Container object
	* @param string	$name			the name of the attribute
	* @param string	$value			the value of the attribute
	*
	* @return boolean
	* @access public
	*/
	function setAttribute(&$attributes, $name, $value)
	{
		$name = strtolower($name);
		if (empty($attributes[$name])) $attributes[$name] = '';
		if ($attributes[$name] == $value) return FALSE;
		$attributes[$name] = $value;
		return TRUE;

	}//end setAttribute()


	/**
	* Prints the JavaScript needed by the backend functions for this table
	*
	* @param object	&$asset	the container object being printed
	* @param string	$prefix	prefix for form fields
	*
	* @return void
	* @access public
	*/
	function paintBackendJsData(&$asset, $prefix)
	{

	}//end paintBackendJsData()


	/**
	* Paint JS data that containers need to use to do their processing
	*
	* @param object	&$bodycopy	the bodycopy being printed
	* @param object	&$o			the outputter class
	* @param string	$prefix		prefix for the form element
	*
	* @return void
	* @access public
	*/
	function paintGenericBackend(&$bodycopy, &$o, $prefix)
	{

	}//end paintGenericBackend()


	/**
	* Paints this container as HTML
	*
	* @param object		&$asset		the container whose interface we are painting
	* @param boolean	$editing	are we printing an editing interface
	* @param boolean	$generating	are we generating the content file
	*
	* @return void
	* @access public
	*/
	function paint(&$asset, $editing=FALSE, $generating=FALSE)
	{

	}//end paint()


	/**
	* Paint the interface for editing this container
	*
	* @param object	&$asset	the container whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintContainer(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('content')) {
			if ($asset->readAccess()) {
				$this->paint($asset, TRUE);
			} else {
				echo '<span class="sq-backend-warning">You do not have access to view the contents of this container</span>';
			}
			return FALSE;
		}
		return TRUE;

	}//end paintContainer()


	/**
	* Process the interface for editing this container
	*
	* @param object	&$asset		the container whose interface we are painting
	* @param object	&$o			the outputter class
	* @param string	$prefix		prefix for the form element
	* @param string	$bc_action	a global bodycopy action to perform
	* @param string	$bc_name	the name of the bodycopy that submitted the action
	* @param array	$bc_data	an array of data needed to perform the action
	* @param array	$bc_saved	any saved changes that were performed without committing
	*
	* @return boolean
	* @access public
	*/
	function processContainer(&$asset, &$o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved)
	{
		$updated = FALSE;

		// save any changed attributes
		if (!empty($bc_data[$prefix][$asset->id]['attributes']) || !empty($bc_saved[$prefix][$asset->id]['attributes'])) {
			$attributes_updated = FALSE;
			$attributes = $asset->attr('attributes');
			if (isset($bc_data[$prefix][$asset->id]['attributes'])) {
				foreach ($bc_data[$prefix][$asset->id]['attributes'] as $name => $value) {
					if ($name == 'identifier') {
						if ($value != '') {
							if ($asset->setAttrValue('name', $value)) {
								$updated = TRUE;
							}
						}
					} else {
						if ($this->setAttribute($attributes, $name, $value)) {
							$attributes_updated = TRUE;
						}
					}
				}
			}
			if (isset($bc_saved[$prefix][$asset->id]['attributes'])) {

				foreach ($bc_saved[$prefix][$asset->id]['attributes'] as $name => $value) {
					if ($name == 'identifier') {
						if ($value != '') {
							if ($asset->setAttrValue('name', $value)) {
								$updated = TRUE;
							}
						}
					} else {
						if ($this->setAttribute($attributes, $name, $value)) {
							$attributes_updated = TRUE;
						}
					}
				}
			}
			if ($attributes_updated) {
				$asset->setAttrValue('attributes', $attributes);
				$updated = TRUE;
			}
		}//end if - there are any attributes to be saved

		return $updated;

	}//end processContainer()


	/**
	* Creates a PHP file to print on the frontend instead of doing processing there
	*
	* @param object	&$asset	the container
	*
	* @return boolean
	* @access public
	*/
	function generateContentFile(&$asset)
	{
		ob_start();
			$this->paint($asset, FALSE, TRUE);
			$output = ob_get_contents();
		ob_end_clean();

		if (!$this->createContentLinks($asset, $output)) {
			return FALSE;
		}

		// replacing normal assetids is simple because they can only be numerical chars :)
		$e = '|\\./\\?a=([0-9]+)([^0-9:])|';
		$r = '<'.'?php echo array_get_index($sq_asset_url_list, $1, "./?a=$1"); ?'.'>$2';
		$output = preg_replace($e, $r, $output);
		// replacing shadow assetids isn't so simple because they can have any characters...and therefore need to be escaped
		$e = '|\\./\\?a=([0-9]+:[^$]*)\\$|';
		if (preg_match_all($e, $output, $matches=Array(), PREG_PATTERN_ORDER)) {

			$replacements = Array();
			foreach ($matches[1] as $shadow_assetid) {
				// if this is a shadow asset, then we need to escape the shadow part (as it is plain text)
				$replacements[] = '<'.'?php echo $sq_asset_url_list['.var_export($shadow_assetid, 1).']; ?'.'>';
			}

			$output = str_replace($matches[0], $replacements, $output);

		}// end if

		require_once SQ_FUDGE_PATH.'/general/text.inc';
		$keywords = retrieve_keywords_replacements($output);

		$attributes = $asset->attr('attributes');
		if (empty($attributes['disable_keywords'])) {
			foreach ($keywords as $keyword) {
				$output = preg_replace('|%((<([^>]+?)>)+)?'.$keyword.'((<([^>]+?)>)+)?%|', '\\1<'.'?php echo (isset($keyword_replacements["'.$keyword.'"])) ? $keyword_replacements["'.$keyword.'"] : ""; ?'.'>\\4', $output);
			}
		} else {
			// if 'Disable Keywords' is checked, change '%' sign to '&#37;'
			// so that keywords get printed out correctly on the frontend, and replacements at higher level are not affected
			// content file is generated each time the user commits, so we dont have to bother to convert the special char back
			foreach ($keywords as $keyword) {
				$output = preg_replace('|%((<([^>]+?)>)+)?'.$keyword.'((<([^>]+?)>)+)?%|', '\\1<'.'?php echo "&#37;'.$keyword.'&#37;"; ?'.'>\\4', $output);
			}
		}

		// php removes new line character if closing php tag is before it
		// add another new line character at the end of the line if line ends with php closing tag
		$output = str_replace("\r\n","\n", $output);
		$output = str_replace("\r","\n", $output);
		$output = preg_replace("/\\?>\n/m", '?'.'>'."\n\n", $output);

		create_directory($asset->data_path);
		return string_to_file($output, $asset->data_path.'/content_file.php');

	}//end generateContentFile()


	/**
	* Creates and deletes NOTICE links based on the assets being linked to within the contents of this container
	* For shadow assets it
	*
	* @param object			&$asset	the container
	* @param string|NULL	$output	the content of the bodycopy container,
	*								leave NULL to have it generated for you
	*
	* @return boolean
	* @access public
	*/
	function createContentLinks(&$asset, $output=NULL)
	{
		if (is_null($output)) {
			ob_start();
				$this->paint($asset, TRUE);
				$output = ob_get_contents();
			ob_end_clean();
		}

		$existing_links = $asset->attr('shadow_links');
		$notice_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE);
		foreach ($notice_links as $link_data) {
			$existing_links[] = $link_data['minorid'];
		}

		// only matches './?a=(normal or shadow id)'
		$shadow_reg = '|\./\?a=(\d+:\w*)|';
		$normal_reg = '|\./\?a=(\d+)|';
		preg_match_all($shadow_reg, $output, $shadow_matches=Array());
		preg_match_all($normal_reg, $output, $normal_matches=Array());
		$shadow_matches = $shadow_matches[1];
		$normal_matches = $normal_matches[1];
		$replace_assetids = Array();
		foreach ($shadow_matches as $data) {
			$replace_assetids[] = $data;
		}
		foreach ($normal_matches as $data) {
			$replace_assetids[] = $data;
		}
		$replace_assetids = array_unique($replace_assetids);

		if ($asset->status & SQ_SC_STATUS_SAFE_EDITING) {
			// this container currently has a system version floating around
			// so we have to be careful not to delete notice links that are
			// still being used in this system version
			require_once SQ_FUDGE_PATH.'/general/file_system.inc';
			$system_assetids = unserialize(file_to_string($asset->data_path.'/.sq_system/.notice_links'));
			$replace_assetids = array_merge($replace_assetids, $system_assetids);
		}

		if (!empty($replace_assetids) || !empty($notice_links)) {

			$replace_assetids = array_unique($replace_assetids);

			// delete existing notice links that are no longer being used (only delete those without a link value)
			$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
			$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');
			foreach ($notice_links as $link_data) {
				// if the link value doesn't indicate a thumbnail, or tag weighting, etc.
				// NOTE: This needs to be refactored.
				if (empty($link_data['value'])) {
					if (!in_array($link_data['minorid'], $replace_assetids)) {
						if (!$asset->deleteLink($link_data['linkid'])) {
							$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
							$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
							return FALSE;
						}
					}
				}
			}

			$shadow_links = array_values(array_intersect($asset->attr('shadow_links'), $replace_assetids));
			foreach ($replace_assetids as $assetid) {
				if (!assert_valid_assetid($assetid, '', FALSE, FALSE)) {
					continue;
				}
				if (in_array($assetid, $existing_links)) continue;

				$ereg_asset =& $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				if (is_null($ereg_asset)) continue;

				if (strpos($ereg_asset->id, ':') !== FALSE) {
					$shadow_links[] = $ereg_asset->id;

				} else {

					// dont show errors here if the link already exists
					// as we rely on the fact that this ereg_asset cant be notice linked
					// to asset mutiple times to stop redunant links from
					// being created (eg same image on a page 10 times should only
					// produce one notice link)
					$asset->createLink($ereg_asset, SQ_LINK_NOTICE);
				}

			}//end foreach

			if (!$asset->setAttrValue('shadow_links', $shadow_links) || !$asset->saveAttributes()) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();


		}//end if replace_assetids or notice_links

		return TRUE;

	}//end createContentLinks()


	/**
	* gets called when a bodycopy container's contents are updated to notify anyone who cares about this
	*
	* @param object	&$asset	the asset that is getting updated
	*
	* @return boolean
	* @access public
	*/
	function _contentsUpdated(&$asset)
	{
		if (is_null($asset)) return FALSE;
		$em =& $GLOBALS['SQ_SYSTEM']->getEventManager();
		return $em->broadcastEvent($asset, 'contentsUpdated', Array());

	}//end _contentsUpdated()


	/**
	* When this bodycopycontainer is painted on its own without being a part
	* of the 'contents' of a bodycopy, draw the editing info by itself. Most of
	* the code is directly taken from the Bodycopy_Edit_Fns class as the
	* functionality is so similar. Perhaps going to aggregate it out to
	* a separate class though once it is understood better.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintBodycopyContainer(&$asset, &$o, $prefix)
	{
		$am =& $GLOBALS['SQ_SYSTEM']->am;

		// if there's a preview url set for this asset, get the design info for
		// it.
		if (isset($GLOBALS['sq_preview_url']) && !empty($GLOBALS['sq_preview_url'])) {
			$url = $GLOBALS['sq_preview_url'];
			$url = preg_replace('|^http[s]?://|', '', $url);

			$db =& $GLOBALS['SQ_SYSTEM']->db;
			$sql = 'SELECT value
					FROM '.SQ_TABLE_RUNNING_PREFIX.'ast_lookup_value ';
			$where = '    url = '.$db->quote($url).'
					  AND name LIKE '.$db->quote('design::%');
			$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where);

			$designid = $db->getOne($sql.$where);
			assert_valid_db_result($designid);
			if ($designid) {
				if (!SQ_IN_LIMBO) {
					$design =& $am->getAsset($designid);
					$stylesheets = $design->getStylesheets();

					// save the ids of all the css files into a global var so the wysiwyg style
					// drop down plugin can access this information without another querey
					// we can also limit the stylesheets that are available to it here if we want
					$GLOBALS['sq_bodycopy_cssids'] = array_keys($stylesheets);

					$css_urls = $am->getAssetURL($GLOBALS['sq_bodycopy_cssids']);
					?>
					<style type="text/css">
						/* import styles from the design for this asset */
						<?php
						foreach ($css_urls as $cssid => $css_url) {
							?>@import url("<?php echo $css_url; ?>");<?php
						}
						?>
					</style>
					<?php
					$am->forgetAsset($design);
					unset($design);
				}
			} else {
				?>
				<p class="sq-backend-notice"><b>NOTE:</b> This asset does not have a design applied to it for the URL currently being edited [<b><?php echo $GLOBALS['sq_preview_url']; ?></b>]. No style information is available to use for content editing.</p>
				<?php
			}
		} else {
			?>
			<p class="sq-backend-notice"><b>NOTE:</b> This asset does not have a preview URL or design applied to it in this context. No style information is available to use for content editing.</p>
			<?php
		}

		// if there's no write access to the content
		if (!$asset->writeAccess('content')) {
			if ($asset->useSystemVersion()) {
				$asset->printBody();
			} else {
				$this->paintContainer($asset, $o, $prefix);
			}
			return FALSE;
		}

		// the initial width and height to have the layer display created at;
		$init_layer_width  = 500;
		$init_layer_height = 500;

		// all available content types
		$content_types = Array();
		$types = $am->getAssetTypeHierarchy('content_type');
		foreach ($types as $type => $type_data) {
			$content_types[$type] = Array('name' => str_replace(' Content Type', '', $type_data['name']));
		}

		?>
		<input type="hidden" name="bodycopy_action" value="">
		<input type="hidden" name="bodycopy_name"   value="">
		<input type="hidden" name="bodycopy_data"   value="">

		<script type="text/javascript"src="<?php echo sq_web_path('fudge').'/var_serialise/var_serialise.js'?>"	></script>
		<script type="text/javascript"src="<?php echo sq_web_path('lib').'/js/detect.js';?>"					></script>
		<script type="text/javascript"src="<?php echo sq_web_path('lib').'/js/layer_handler.js';?>"				></script>
		<script type="text/javascript"src="<?php echo sq_web_path('lib').'/js/general.js';?>"					></script>
		<script type="text/javascript"src="<?php echo sq_web_path('lib').'/js/debug.js';?>"						></script>
		<script type="text/javascript"src="<?php echo sq_web_path('lib').'/js/tooltip.js';?>"					></script>
		<script type="text/javascript"src="<?php echo sq_web_path('lib').'/html_form/html_form.js';?>"			></script>

		<script type="text/javascript">
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

			var bodycopy_initialised = false;	// TRUE once init() has been run
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
			}//end bodycopy_submit()

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
					if (confirm(js_translate('page_not_loaded_yet'))) {
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
					// frameElement is IE 5.5 only
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
			}//end bodycopy_show_popup()

			function bodycopy_hide_popup() {
				if (!bodycopy_initialised) { return; }
				bodycopy_show_popup("blank.php");
				bodycopy_popup_visible = null;
				bodycopy_popup.hide();
			}//end bodycopy_hide_popup()

			// get the available cell types
			var BODYCOPY_AVAILABLE_CONTENT_TYPES = null;
			function get_bodycopy_available_content_types() {
				if (BODYCOPY_AVAILABLE_CONTENT_TYPES == null) {
					BODYCOPY_AVAILABLE_CONTENT_TYPES = var_unserialise('<?php echo var_serialise($content_types, TRUE)?>');
				}
				return BODYCOPY_AVAILABLE_CONTENT_TYPES;
			}

			function bodycopy_data_exists(args) {
				var str = 'bodycopy_saved';
				for (var i = 0; i < args.length; i++) {
					switch (typeof(args[i])) {
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

			bodycopy_current_data["<?php echo $prefix?>"] = new Object();
		</script>

		<?php
		// print JS functions for each of the container types we have installed
		$container_types = $am->getAssetTypeHierarchy('bodycopy_container');
		foreach ($container_types as $type => $type_data) {
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type);
			$type_asset =& new $type();
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
			<tr>
				<td>
					<input type="hidden" name="bodycopy_saved[<?php echo $prefix?>][<?php echo $asset->id?>]" value="">
					<?php $this->paintContainer($asset, $o, $prefix); ?>
				</td>
			</tr>
		</table>
		<?php

		return TRUE;

	}//end paintBodycopyContainer()


	/**
	* When this bodycopy_container is painted on its own without being a part
	* of the 'contents' of a bodycopy, process it separately than normal.
	*
	* @param object	&$asset	the container whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return void
	* @access public
	*/
	function processBodycopyContainer(&$asset, &$o, $prefix)
	{
		$updated = FALSE;
		$bc_action = (empty($_POST['bodycopy_action'])) ? ''      : $_POST['bodycopy_action'];
		$bc_name   = (empty($_POST['bodycopy_name']))   ? ''      : $_POST['bodycopy_name'];
		$bc_data   = (empty($_POST['bodycopy_data']))   ? Array() : $_POST['bodycopy_data'];
		$bc_saved  = (empty($_POST['bodycopy_saved']))  ? Array() : $_POST['bodycopy_saved'];

		// if there is some bodycopy data, deserialise it into the arrays
		// so that everything else can use it
		if ($bc_data) $bc_data = var_unserialise($bc_data);

		// unserialize the data for a bodycopy.
		for (reset($bc_saved); NULL !== ($k = key($bc_saved)); next($bc_saved)) {
			$containers =& $bc_saved[$k];
			for (reset($containers); NULL !== ($containersid = key($containers)); next($containers)) {
				$containers[$containersid] = var_unserialise($containers[$containersid]);
			}
		}

		unset($containers);

		if ($this->processContainer($asset, $o, $prefix, $bc_action, $bc_name, $bc_data, $bc_saved)) {
			if ($asset->saveAttributes()) $updated = TRUE;
		}
		return $updated;

	}//end processBodycopyContainer()


}//end class

?>

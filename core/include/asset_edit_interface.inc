<?php

require_once 'XML/Tree.php';

/**
* Asset_Edit_Interface
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve
*/
class Asset_Edit_Interface extends Resolve_Object
{

	/**
	* The asset type that we are creating the interface for
	* @var string
	*/
	var $_type_code = '';

	/**
	* The Edit Fns object for dealing with any complex attribute painting and processing
	* @var object Asset_Edit_Fns
	*/
	var $_edit_fns = null;

	/**
	* An array of dirs to check for certain files for this asset type
	* @var Array()
	*/
	var $_dirs = Array();

	/**
	* An array of screen code names and their display names
	* @var Array()
	*/
	var $_screens = Array();

	/**
	* The code name for default screen for the interface - used when no existing screen is specified
	* @var string
	*/
	var $_default_screen = '';

	/**
	* Constructor
	*
	* @param string	$type_code
	*
	* @access  public
	*/
	function Asset_Edit_Interface($type_code)
	{
		$this->_type_code = $type_code;

		$asset_dir = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_RESOLVE']->am->getInfo($this->_type_code, 'dir');
		$class_name = $this->_type_code.'_edit_fns';
		include_once $asset_dir.'/'.$class_name.'.inc';
		$this->_edit_fns = new $class_name();

		$this->_dirs = Array($asset_dir);
		$parents = $GLOBALS['SQ_RESOLVE']->am->getParentList($this->_type_code, false);
		foreach($parents as $parent_type) {
			$this->_dirs[] = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_RESOLVE']->am->getInfo($parent_type, 'dir');
		}

		// add the default for Asset
		$this->_dirs[] = SQ_INCLUDE_PATH.'/asset_edit';

		if(!($file = $this->_findFile('screens'))) return;

		$input = new XML_Tree($file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		$this->_screens = Array();
		for($i = 0; $i < count($root->children); $i++) {
			$code_name = strtolower(preg_replace('/[^a-z_]/', '', $root->children[$i]->attributes['code_name']));
			if (!$code_name) continue;
			$this->_screens[$code_name] = Array(
												'name' => $root->children[$i]->attributes['display_name'],
												'icon' => $root->children[$i]->attributes['icon']
											);
			if (!empty($root->children[$i]->attributes['default'])) $this->_default_screen = $code_name;
		}// end for

	}// end constructor;


	/**
	* Returns the screens array
	*
	* @access public
	*/
	function getScreens()
	{
		return $this->_screens;
	}


	/**
	* Paint's out the interface for the currently selected frame
	*
	* @param object	Asset				$owner		the asset whose interface we are painting
	* @param object	Backend_Outputter	$o			the outputter class
	* @param boolean					$creating	whether we are creating or editing
	*
	* @access  public
	*/
	function paint(&$owner, &$o, $creating)
	{
		// if there are no screens, piss off
		if (!count($this->_screens)) return;

		$current_location = $o->getCurrentLocation();
		$current_location .= (strstr($current_location, '?')) ? '&' : '?';

		// if there are no screens, piss off
		for(reset($this->_screens); null !== ($code = key($this->_screens)); next($this->_screens)) {
			$o->addScreen($current_location.'asset_ei_screen='.rawurlencode($code), $this->_screens[$code]['name']);
		}// end for

		$create_sections = Array();
		$active_screen = (empty($_GET['asset_ei_screen'])) ? $this->_default_screen : $_GET['asset_ei_screen'];

		$o->setCurrentScreen($current_location.'asset_ei_screen='.rawurlencode($active_screen));

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_screen) = $this->_getCreateSections();
			if (!count($create_sections)) return;
		}// end if create

		$active_screen = (empty($_GET['asset_ei_screen'])) ? $this->_default_screen : $_GET['asset_ei_screen'];

		if (empty($this->_screens[$active_screen])) {
			trigger_error('Active Screen ("'.$active_screen.'") unknown', E_USER_WARNING);
			return;
		}

		// try and find the file
		if(!($file = $this->_findFile('screen_'.$active_screen))) return;

		// make sure that we have the screen for next time
		$o->addFormActionGetVar('asset_ei_screen', $active_screen);

		$o->setHeading($this->_screens[$active_screen]['name'], $this->_screens[$active_screen]['icon']);

		$input = new XML_Tree($file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		// prefix for all the form vars
		$prefix = $owner->type().'_'.$owner->id;

		// Loop over the sections
		for($i = 0; $i < count($root->children); $i++) {
			$section = &$root->children[$i];
			// if this ain't a section, or if we are printing the create interface and this section is not included in that, ignore it
			if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) continue;

			$o->openSection($section->attributes['name']);
			// Loop over the fields
			for($j = 0; $j < count($section->children); $j++) {
				$field = &$section->children[$j];
				// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
				if ($field->name != 'field'  || ($creating && !in_array($field->attributes['attribute'], $create_sections[$section->attributes['name']]))) continue;

				if ($field->children[0]->name != 'display_name') {
					trigger_error('Display Name for "'.$field->attributes['attribute'].'" not found', E_USER_WARNING);
					continue;
				}

				$o->openField($field->children[0]->content);

				if (!empty($field->attributes['attribute'])) {
					$attr = &$owner->getAttribute($field->attributes['attribute']);
					if (is_null($attr)) continue;
					if (!$attr->setEditParams($field->children[2])) continue;
					$attr->paint($prefix);

				} else if ($field->children[2]->name == 'function_call') {

					$paint_node = &$field->children[2]->children[0];

					if (empty($paint_node->attributes['name']) || !method_exists($this->_edit_fns, $paint_node->attributes['name'])) {
						echo 'Function not found for painting';
					} else {
						$this->_edit_fns->{$paint_node->attributes['name']}($owner, $o, $prefix);
					}

				}// end if

				if ($field->children[1]->name == 'note' && $field->children[1]->content) {
					$o->note($field->children[1]->content);
				}

			}// end for

		}// end for

		$o->commitButton('Commit');

	}// end paint();


	/**
	* Processes the results of the form posting, returns an array of the attribute names that
	* where processed
	*
	* @param object Asset	$owner		the asset whose interface we are painting
	* @param boolean		$creating	whether we are creating or editing
	*
	* @return Array()
	* @access public
	*/
	function process(&$owner, $creating)
	{
		// if there are no screens, piss off
		if (!count($this->_screens)) return;

		$processed = Array();
		$create_sections = Array();
		$active_screen = (empty($_GET['asset_ei_screen'])) ? $this->_default_screen : $_GET['asset_ei_screen'];

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_screen) = $this->_getCreateSections();
			if (!count($create_sections)) return $processed;
		}// end if create

		if (empty($_GET['asset_ei_screen']) || empty($this->_screens[$_GET['asset_ei_screen']])) {
			trigger_error('Active Screen unknown unable to process', E_USER_WARNING);
			return $processed;
		}

		$file = '';
		for($i = 0; $i < count($this->_dirs); $i++) {
			if (file_exists($this->_dirs[$i].'/edit_interface_screen_'.$active_screen.'.xml')) {
				$file = $this->_dirs[$i].'/edit_interface_screen_'.$active_screen.'.xml';
				break;
			}
		}

		if(!$file) {
			trigger_error('edit_interface_screens.xml not found for Asset Type "'.$this->_type_code.'"', E_USER_WARNING);
			return $processed;
		}

		$input = new XML_Tree($file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return $processed;
		}

		// prefix for all the form vars
		$prefix = $owner->type().'_'.$owner->id;
		// Loop over the sections
		for($i = 0; $i < count($root->children); $i++) {
			$section = &$root->children[$i];
			// if this ain't a section, or if we are printing the create interface and this section is not included in that, ignore it
			if ($section->name != 'section' || ($creating && empty($create_sections[$section->attributes['name']]))) continue;

			// Loop over the fields
			for($j = 0; $j < count($section->children); $j++) {
				$field = &$section->children[$j];
				// if this ain't a field, or if we are printing the create interface and this field is not included in that, ignore it
				if ($field->name != 'field'  || ($creating && !in_array($field->attributes['attribute'], $create_sections[$section->attributes['name']]))) continue;

				if (!empty($field->attributes['attribute'])) {
					$attr = &$owner->getAttribute($field->attributes['attribute']);
					if (is_null($attr)) continue;
					if (!$attr->setEditParams($field->children[2])) continue;

					$attr->process($prefix);
					$processed[] = $attr->name;

				} else if ($field->children[2]->name == 'function_call') {

					$process_node = &$field->children[2]->children[1];

					// if there is nothing declared there is nothing for use to do
					if (empty($process_node->attributes['name'])) {
						continue;
					// There is something declared but the bloody thing doesn't exist - inform the Idiot
					} elseif (!method_exists($this->_edit_fns, $process_node->attributes['name'])) {
						trigger_error('Function not found for processing field "'.$field->children[0]->content.'"', E_USER_WARNING);
					} else {
						$this->_edit_fns->{$process_node->attributes['name']}($owner, $o, $prefix);
					}

				}// end if

			}// end for

		}// end for

		return $processed;

	}// end process();


	/**
	* Takes the requested filename and searches back up the directory's to find the matching file
	* closest to the type of the asset that we are creating
	*
	* @param string	$file_name	the file to find
	*
	* @return string
	* @access private
	*/
	function _findFile($file_name)
	{

		$file_name = 'edit_interface_'.$file_name.'.xml';

		$file = '';
		for($i = 0; $i < count($this->_dirs); $i++) {
			if (file_exists($this->_dirs[$i].'/'.$file_name)) {
				$file = $this->_dirs[$i].'/'.$file_name;
				break;
			}
		}

		if(!$file) {
			trigger_error($file_name.' not found for Asset Type "'.$this->_type_code.'"', E_USER_WARNING);
			return false;
		}

		return $file;
	}// end _findFile()

	/**
	* Returns the Sections and fields that are to be used in a create interface
	* and the screen that these belong to
	*
	* @return Array
	* @access private
	*/
	function _getCreateSections()
	{

		$create_sections = Array();

		if (!($create_file = $this->_findFile('create'))) return $create_sections;

		$input = new XML_Tree($create_file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return $create_sections;
		}

		// Loop over the sections
		for($i = 0; $i < count($root->children); $i++) {
			$section = &$root->children[$i];
			if ($section->name != 'section') continue;

			$create_sections[$section->attributes['name']] = Array();
			// Loop over the fields
			for($j = 0; $j < count($section->children); $j++) {
				$field = &$section->children[$j];
				if ($field->name != 'field') continue;
				$create_sections[$section->attributes['name']][] = $field->attributes['attribute'];

			}// end for

		}// end for

		return Array($create_sections, $root->attributes['screen']);

	}// end _getCreateSections()

}//end class


?>

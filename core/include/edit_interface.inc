<?php

require_once 'XML/Tree.php';

/**
* Edit_Interface
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Edit_Interface extends Resolve_Object
{

	/**
	* The asset type that we are creating the interface for
	* @var string
	*/
	var $_type_code = '';

	/**
	* An array of dirs to check for certain files for this asset type
	* @var Array()
	*/
	var $_dirs = Array();

	/**
	* An array of tab code names and their display names
	* @var Array()
	*/
	var $_tabs = Array();

	/**
	* The code name for default tab for the interface - used when no existing tab is specified
	* @var string
	*/
	var $_default_tab = '';
	 
	/**
	* Constructor
	*
	* @param string	$type_code
	*
	* @access  public
	*/
	function Edit_Interface($type_code)
	{
		$this->_type_code = $type_code;

		$this->_dirs = Array(SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_RESOLVE']->am->getInfo($this->_type_code, 'dir'));
		$parents = $GLOBALS['SQ_RESOLVE']->am->getParentList($this->_type_code, false);
		foreach($parents as $parent_type) {
			$this->_dirs[] = SQ_SYSTEM_ROOT.'/'.$GLOBALS['SQ_RESOLVE']->am->getInfo($parent_type, 'dir');
		}

		$this->_loadTabs();
	}

	/**
	* Looks for an 'edit_interface_tabs.xml' file in the base dir and loads it's information
	*
	* @access  private
	*/
	function _loadTabs() 
	{

		if(!($file = $this->_findFile('tabs'))) return;

		$input = new XML_Tree($file);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error($root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}

		$this->_tabs = Array();
		for($i = 0; $i < count($root->children); $i++) {
			$code_name = strtolower(preg_replace('/[^a-z_]/', '', $root->children[$i]->attributes['code_name']));
			if (!$code_name) continue;
			$this->_tabs[$code_name] = Array(
												'name' => $root->children[$i]->attributes['display_name'],
												'icon' => $root->children[$i]->attributes['icon']
											);
			if (!empty($root->children[$i]->attributes['default'])) $this->_default_tab = $code_name;
		}

	}// end _loadTabs();


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
		// if there are no tabs, piss off
		if (!count($this->_tabs)) return;


		$create_sections = Array();
		$active_tab = (empty($_GET['edit_interface_tab'])) ? $this->_default_tab : $_GET['edit_interface_tab'];

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_tab) = $this->_getCreateSections();
			if (!count($create_sections)) return;
		}// end if create

		$active_tab = (empty($_GET['edit_interface_tab'])) ? $this->_default_tab : $_GET['edit_interface_tab'];

		if (empty($this->_tabs[$active_tab])) {
			trigger_error('Active Tab ("'.$active_tab.'") unknown', E_USER_WARNING);
			return;
		}

		// try and find the file
		if(!($file = $this->_findFile('tab_'.$active_tab))) return;

		// make sure that we have the tab for next time
		$o->addFormActionGetVar('edit_interface_tab', $active_tab);

		$o->setHeading($this->_tabs[$active_tab]['name'], $this->_tabs[$active_tab]['icon']);

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

				$attr = &$owner->getAttribute($field->attributes['attribute']);
				if (is_null($attr)) continue;

				if ($field->children[0]->name != 'display_name') {
					trigger_error('Display Name for "'.$field->attributes['attribute'].'" not found', E_USER_WARNING);
					continue;
				}

				if (!$attr->setEditParams($field->children[2])) continue;

				$o->openField($field->children[0]->content);

				$attr->paint($prefix);

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
		// if there are no tabs, piss off
		if (!count($this->_tabs)) return;

		$processed = Array();
		$create_sections = Array();
		$active_tab = (empty($_GET['edit_interface_tab'])) ? $this->_default_tab : $_GET['edit_interface_tab'];

		// if we are creating open the create file to see which fields we are supposed to print out
		if ($creating) {
			list($create_sections, $active_tab) = $this->_getCreateSections();
			if (!count($create_sections)) return $processed;
		}// end if create

		if (empty($_GET['edit_interface_tab']) || empty($this->_tabs[$_GET['edit_interface_tab']])) {
			trigger_error('Active Tab unknown unable to process', E_USER_WARNING);
			return $processed;
		}

		$file = '';
		for($i = 0; $i < count($this->_dirs); $i++) {
			if (file_exists($this->_dirs[$i].'/edit_interface_tab_'.$active_tab.'.xml')) {
				$file = $this->_dirs[$i].'/edit_interface_tab_'.$active_tab.'.xml';
				break;
			}
		}

		if(!$file) {
			trigger_error('edit_interface_tabs.xml not found for Asset Type "'.$this->_type_code.'"', E_USER_WARNING);
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

				$attr = &$owner->getAttribute($field->attributes['attribute']);
				if (is_null($attr)) continue;
				if (!$attr->setEditParams($field->children[2])) continue;

				$attr->process($prefix);
				$processed[] = $attr->name;

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
	* @access public
	*/
	function _findFile($file_name) {

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
	* and the tab that these belong to
	*
	* @return Array
	* @access public
	*/
	function _getCreateSections() {

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

		return Array($create_sections, $root->attributes['tab']);

	}// end _getCreateSections()

}//end class


?>

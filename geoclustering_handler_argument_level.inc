<?php
// $Id

class geoclustering_handler_argument_level
									extends views_handler_argument
{
	function pre_query()
		{
			$this->geoclustering_tree_name=
								$this->options['geoclustering_tree_name'];

			// Set default argument here, because if validation of some
			//   other argument fails, query() will never be called
			if ($this->has_default_argument()) {
				$this->geoclustering_level=$this->get_default_argument();
				}

			//!!! max_results parameeter lisada
			}

	function query()
		{
				// Actual query modification for this argument happens in
				//   geoclustering_views_query_alter(), not here, because
				//   it rewrites the query fully and should therefore be
				//   done as late in query building process as possible.
				//   So here we just record the argument value for
				//   geoclustering_views_query_alter() to react on.

			$this->geoclustering_level=$this->argument;
			}

	function option_definition()
		{
			$options=parent::option_definition();
			$options['default_action']['default']='default';
			$options['default_argument_type']['default']='auto';
			$options['validate_type']['default']='numeric';
			return $options;
			}

	function options_form(&$form,&$form_state)
		{
			parent::options_form($form,$form_state);

			$tree_name_options=array();
			ctools_include('export');
			foreach (ctools_export_load_object('geoclustering_tree_params')
													as $name => $params) {
				$tree_name_options[$name]=isset($params->description) ?
											$params->description : $name;
				}

			$tree_name_element=array(
						'#title' => t('Geoclustering tree'),
						'#type' => 'select',
						'#options' => $tree_name_options,
						'#multiple' => FALSE,
						'#weight' => -3,
						);

			if (empty($tree_name_options)) {
				$tree_name_element['#description']=t(
						'There are no Geoclustering trees defined. ' .
						'You need to define at least one tree (using ' .
						'hook_geoclustering_tree_params()) before you ' .
						'can use them in Views');
				}

			$form['geoclustering_tree_name']=$tree_name_element;

			$form['default_action']['#weight']=-2;
			$form['default_argument_type']['#weight']=-1;
			}

	function options_validate($form,&$form_state)
		{
			parent::options_validate($form,$form_state);

			if ($form_state['values']['form_id'] ==
											'views_ui_config_item_form' &&
						!$form_state['values']['options']
											['geoclustering_tree_name']) {
				form_error($form['geoclustering_tree_name'],
						t('You must select a Geoclustering tree ' .
													'for this argument'));
				}
			}

	function default_actions($which=NULL)
		{
			$defaults=parent::default_actions();

			unset($defaults['summary asc']);
			unset($defaults['summary desc']);

			if ($which) {
				if (!empty($defaults[$which])) {
					return $defaults[$which];
					}
				}
			else {
				return $defaults;
				}
			}

	function default_argument_form(&$form,&$form_state)
		{
			parent::default_argument_form($form,$form_state);
			$form['default_argument_type']['#options']+=array(
					'auto' =>
						t('Auto-select level based on node density in given area'),
					'nodes' =>
						t('Nodes level (tree leaves, no clusters)'),
					);
			foreach (array('node','user','current_user') as $key) {
				unset($form['default_argument_type']['#options'][$key]);
				}
			}

	function get_default_argument($raw=FALSE)
		{
			if (!$raw && $this->options['default_argument_type'] == 'auto') {
				return -1;
				}
			if (!$raw && $this->options['default_argument_type'] == 'nodes') {
				return -2;
				}

			//!!! support "auto" or "nodes" as argument value

			return parent::get_default_argument($raw);
			}
	}

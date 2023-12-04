#pragma once

class Debuggable
{
public:
	std::string debug_name = "unnamed debug object";
	virtual const char* get_name() const { return debug_name.c_str(); };
	virtual const char* get_type_name() const = 0;

	virtual ~Debuggable() {};

	template <class T>
	static void print_debug_object(const T* object)
	{
		check(object != nullptr);
		if (const Debuggable* d = dynamic_cast<const Debuggable*>(object))
		{
			//printf("Debug type:")
			printf("Debug type: %s, Debug name: %s\n", d->get_type_name(), d->get_name());
		}
		else
		{
			printf("Debug name: NULL\n");
		}
	}
};
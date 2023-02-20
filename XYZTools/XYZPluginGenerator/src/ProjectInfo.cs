using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace XYZPluginGenerator
{
    internal class ProjectInfo
    {
        public string ProjectName;

        public string Language;

        public string Kind;
        
        public ProjectInfo(object project, Type type)
        {
            ExtractFields(project, type);
        }

        public Dictionary<string, string> ToDictionary()
        {
            var result = new Dictionary<string, string>();
            Type thisType = GetType();
            foreach (var thisField in thisType.GetFields())
            {
                string value = thisField.GetValue(this).ToString();
                result.Add(thisField.Name, value);
            }

            return result;
        }

        public List<string> Validate()
        {
            List<string> errors = new List<string>();
            Type thisType = GetType();
            
            foreach (var thisField in thisType.GetFields())
            {
                if (thisField.IsPublic && thisField.GetValue(this) == null)
                {
                    errors.Add("Missing value " + thisField.Name);
                }
            }
            return errors;
        }

        private void ExtractFields(object project, Type type)
        {
            Type thisType = GetType();
            foreach (var thisField in thisType.GetFields())
            {
                foreach (var field in type.GetFields())
                {
                    if (TryExtract(thisField, project, field))
                        break;
                }
            }
        }

        private bool TryExtract(FieldInfo dstInfo, object srcIntance, FieldInfo srcInfo)
        {
            if (dstInfo.Name == srcInfo.Name && dstInfo.FieldType.Name == srcInfo.FieldType.Name)
            {
                dstInfo.SetValue(this, srcInfo.GetValue(srcIntance));
                return true;
            }
            return false;
        }
    }
}
